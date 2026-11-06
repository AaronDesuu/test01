package com.example.meterlinkind.data.protocol

/** A decoded COSEM `Data` value (IEC 62056-6-2, A-XDR encoded). */
sealed class DlmsData {
    object Null : DlmsData()
    data class Bool(val value: Boolean) : DlmsData()
    data class BitString(val bitCount: Int, val bytes: ByteArray) : DlmsData()
    /** Every signed/unsigned integer type up to 64 bits; [tag] keeps the wire type. */
    data class Integer(val tag: Int, val value: Long) : DlmsData()
    data class Enum(val value: Int) : DlmsData()
    /** [singlePrecision] for float32, which displays at its own precision (239.73, not 239.73001098632812). */
    data class Float(val value: Double, val singlePrecision: Boolean = false) : DlmsData()
    data class OctetString(val bytes: ByteArray) : DlmsData()
    data class VisibleString(val value: String) : DlmsData()
    data class Utf8String(val value: String) : DlmsData()
    data class Bcd(val bytes: ByteArray) : DlmsData()
    data class DateTime(val bytes: ByteArray) : DlmsData()
    data class Array(val items: List<DlmsData>) : DlmsData()
    data class Structure(val items: List<DlmsData>) : DlmsData()

    /** Human-readable rendering used by the UI and logs. */
    fun display(): String = when (this) {
        Null -> "null"
        is Bool -> value.toString()
        is BitString -> bytes.toHex()
        is Integer -> if (tag == Axdr.TAG_UINT64 && value < 0) value.toULong().toString() else value.toString()
        is Enum -> value.toString()
        is Float -> if (singlePrecision) value.toFloat().toString() else value.toString()
        is OctetString -> CosemDateTime.formatOrNull(bytes) ?: bytes.printableAsciiOrNull() ?: bytes.toHex()
        is VisibleString -> value
        is Utf8String -> value
        is Bcd -> bytes.toHex()
        is DateTime -> CosemDateTime.formatOrNull(bytes) ?: bytes.toHex()
        is Array -> items.joinToString(prefix = "[", postfix = "]") { it.display() }
        is Structure -> items.joinToString(prefix = "{", postfix = "}") { it.display() }
    }
}

class AxdrException(message: String) : Exception(message)

object Axdr {
    const val TAG_NULL = 0
    const val TAG_ARRAY = 1
    const val TAG_STRUCTURE = 2
    const val TAG_BOOLEAN = 3
    const val TAG_BIT_STRING = 4
    const val TAG_INT32 = 5
    const val TAG_UINT32 = 6
    const val TAG_OCTET_STRING = 9
    const val TAG_VISIBLE_STRING = 10
    const val TAG_UTF8_STRING = 12
    const val TAG_BCD = 13
    const val TAG_INT8 = 15
    const val TAG_INT16 = 16
    const val TAG_UINT8 = 17
    const val TAG_UINT16 = 18
    const val TAG_INT64 = 20
    const val TAG_UINT64 = 21
    const val TAG_ENUM = 22
    const val TAG_FLOAT32 = 23
    const val TAG_FLOAT64 = 24
    const val TAG_DATE_TIME = 25
    const val TAG_DATE = 26
    const val TAG_TIME = 27

    /** Decodes exactly one `Data` value starting at [offset]. */
    fun decode(buf: ByteArray, offset: Int = 0): DlmsData = Reader(buf, offset).readData()

    /** Decodes [buf] as one `Data` value and nothing more; throws if bytes are left over. */
    fun decodeWhole(buf: ByteArray): DlmsData {
        val reader = Reader(buf)
        val data = reader.readData()
        if (reader.remaining != 0) throw AxdrException("${reader.remaining} bytes left after the value")
        return data
    }

    /** Encodes one `Data` value; [decode] reads it back unchanged. */
    fun encode(data: DlmsData): ByteArray = java.io.ByteArrayOutputStream().also { write(it, data) }.toByteArray()

    private fun write(out: java.io.ByteArrayOutputStream, data: DlmsData) {
        fun bytes(b: ByteArray) = out.write(b, 0, b.size)
        fun number(value: Long, size: Int) = repeat(size) { out.write((value shr (8 * (size - 1 - it))).toInt() and 0xFF) }
        when (data) {
            DlmsData.Null -> out.write(TAG_NULL)
            is DlmsData.Bool -> { out.write(TAG_BOOLEAN); out.write(if (data.value) 1 else 0) }
            is DlmsData.BitString -> { out.write(TAG_BIT_STRING); bytes(encodeLength(data.bitCount)); bytes(data.bytes) }
            is DlmsData.Integer -> { out.write(data.tag); number(data.value, integerSize(data.tag)) }
            is DlmsData.Enum -> { out.write(TAG_ENUM); out.write(data.value and 0xFF) }
            is DlmsData.Float -> if (data.singlePrecision) {
                out.write(TAG_FLOAT32); number(java.lang.Float.floatToIntBits(data.value.toFloat()).toLong(), 4)
            } else {
                out.write(TAG_FLOAT64); number(java.lang.Double.doubleToLongBits(data.value), 8)
            }
            is DlmsData.OctetString -> { out.write(TAG_OCTET_STRING); bytes(encodeLength(data.bytes.size)); bytes(data.bytes) }
            is DlmsData.VisibleString -> data.value.toByteArray(Charsets.US_ASCII).let {
                out.write(TAG_VISIBLE_STRING); bytes(encodeLength(it.size)); bytes(it)
            }
            is DlmsData.Utf8String -> data.value.toByteArray(Charsets.UTF_8).let {
                out.write(TAG_UTF8_STRING); bytes(encodeLength(it.size)); bytes(it)
            }
            is DlmsData.Bcd -> { out.write(TAG_BCD); bytes(data.bytes) }
            // date-time, date and time all decode to DateTime; their size tells them apart.
            is DlmsData.DateTime -> {
                out.write(when (data.bytes.size) { 5 -> TAG_DATE; 4 -> TAG_TIME; else -> TAG_DATE_TIME })
                bytes(data.bytes)
            }
            is DlmsData.Array -> { out.write(TAG_ARRAY); bytes(encodeLength(data.items.size)); data.items.forEach { write(out, it) } }
            is DlmsData.Structure -> { out.write(TAG_STRUCTURE); bytes(encodeLength(data.items.size)); data.items.forEach { write(out, it) } }
        }
    }

    /** Bytes of an integer type on the wire. */
    fun integerSize(tag: Int): Int = when (tag) {
        TAG_INT8, TAG_UINT8 -> 1
        TAG_INT16, TAG_UINT16 -> 2
        TAG_INT32, TAG_UINT32 -> 4
        TAG_INT64, TAG_UINT64 -> 8
        else -> throw AxdrException("Not an integer tag: $tag")
    }

    /** Values an integer type can hold (unsigned 64-bit is limited to Long's range). */
    fun integerRange(tag: Int): LongRange = when (tag) {
        TAG_INT8 -> Byte.MIN_VALUE.toLong()..Byte.MAX_VALUE.toLong()
        TAG_INT16 -> Short.MIN_VALUE.toLong()..Short.MAX_VALUE.toLong()
        TAG_INT32 -> Int.MIN_VALUE.toLong()..Int.MAX_VALUE.toLong()
        TAG_INT64 -> Long.MIN_VALUE..Long.MAX_VALUE
        TAG_UINT8 -> 0L..0xFFL
        TAG_UINT16 -> 0L..0xFFFFL
        TAG_UINT32 -> 0L..0xFFFF_FFFFL
        TAG_UINT64 -> 0L..Long.MAX_VALUE
        else -> throw AxdrException("Not an integer tag: $tag")
    }

    /** The A-XDR name of a value's type, as Gurux shows it. */
    fun typeName(data: DlmsData): String = when (data) {
        DlmsData.Null -> "null-data"
        is DlmsData.Bool -> "boolean"
        is DlmsData.BitString -> "bit-string"
        is DlmsData.Integer -> when (data.tag) {
            TAG_INT8 -> "integer"; TAG_INT16 -> "long"; TAG_INT32 -> "double-long"; TAG_INT64 -> "long64"
            TAG_UINT8 -> "unsigned"; TAG_UINT16 -> "long-unsigned"; TAG_UINT32 -> "double-long-unsigned"
            else -> "long64-unsigned"
        }
        is DlmsData.Enum -> "enum"
        is DlmsData.Float -> if (data.singlePrecision) "float32" else "float64"
        is DlmsData.OctetString -> "octet-string"
        is DlmsData.VisibleString -> "visible-string"
        is DlmsData.Utf8String -> "utf8-string"
        is DlmsData.Bcd -> "bcd"
        is DlmsData.DateTime -> when (data.bytes.size) { 5 -> "date"; 4 -> "time"; else -> "date-time" }
        is DlmsData.Array -> "array"
        is DlmsData.Structure -> "structure"
    }

    /** A-XDR length: one byte below 0x80, otherwise 0x8N followed by N big-endian bytes. */
    fun encodeLength(length: Int): ByteArray = when {
        length < 0x80 -> byteArrayOf(length.toByte())
        length <= 0xFF -> byteArrayOf(0x81.toByte(), length.toByte())
        else -> byteArrayOf(0x82.toByte(), (length shr 8).toByte(), length.toByte())
    }

    class Reader(private val buf: ByteArray, var pos: Int = 0) {
        val remaining: Int get() = buf.size - pos

        fun u8(): Int {
            if (pos >= buf.size) throw AxdrException("Unexpected end of data at $pos")
            return buf[pos++].toInt() and 0xFF
        }

        fun bytes(n: Int): ByteArray {
            if (n < 0 || pos + n > buf.size) throw AxdrException("Need $n bytes at $pos, have $remaining")
            return buf.copyOfRange(pos, pos + n).also { pos += n }
        }

        fun uint(n: Int): Long {
            var v = 0L
            repeat(n) { v = (v shl 8) or u8().toLong() }
            return v
        }

        private fun sint(n: Int): Long {
            val shift = 64 - 8 * n
            return (uint(n) shl shift) shr shift
        }

        fun length(): Int {
            val first = u8()
            if (first < 0x80) return first
            val n = first and 0x7F
            if (n == 0 || n > 4) throw AxdrException("Bad length prefix 0x%02x".format(first))
            return uint(n).toInt()
        }

        fun readData(): DlmsData = when (val tag = u8()) {
            TAG_NULL -> DlmsData.Null
            TAG_ARRAY -> DlmsData.Array(List(length()) { readData() })
            TAG_STRUCTURE -> DlmsData.Structure(List(length()) { readData() })
            TAG_BOOLEAN -> DlmsData.Bool(u8() != 0)
            TAG_BIT_STRING -> length().let { bits -> DlmsData.BitString(bits, bytes((bits + 7) / 8)) }
            TAG_INT8 -> DlmsData.Integer(tag, sint(1))
            TAG_INT16 -> DlmsData.Integer(tag, sint(2))
            TAG_INT32 -> DlmsData.Integer(tag, sint(4))
            TAG_INT64 -> DlmsData.Integer(tag, sint(8))
            TAG_UINT8 -> DlmsData.Integer(tag, uint(1))
            TAG_UINT16 -> DlmsData.Integer(tag, uint(2))
            TAG_UINT32 -> DlmsData.Integer(tag, uint(4))
            TAG_UINT64 -> DlmsData.Integer(tag, uint(8))
            TAG_ENUM -> DlmsData.Enum(u8())
            TAG_FLOAT32 -> DlmsData.Float(java.lang.Float.intBitsToFloat(uint(4).toInt()).toDouble(), singlePrecision = true)
            TAG_FLOAT64 -> DlmsData.Float(java.lang.Double.longBitsToDouble(uint(8)))
            TAG_OCTET_STRING -> DlmsData.OctetString(bytes(length()))
            TAG_VISIBLE_STRING -> DlmsData.VisibleString(String(bytes(length()), Charsets.US_ASCII))
            TAG_UTF8_STRING -> DlmsData.Utf8String(String(bytes(length()), Charsets.UTF_8))
            TAG_BCD -> DlmsData.Bcd(bytes(1))
            TAG_DATE_TIME -> DlmsData.DateTime(bytes(12))
            TAG_DATE -> DlmsData.DateTime(bytes(5))
            TAG_TIME -> DlmsData.DateTime(bytes(4))
            else -> throw AxdrException("Unsupported data tag $tag at ${pos - 1}")
        }
    }
}

/** COSEM date-time (12-byte octet-string, IEC 62056-6-2 §4.1.6.1). */
object CosemDateTime {
    /** Formats a 12-byte date-time as `yyyy-MM-dd HH:mm:ss`; null if it isn't one. */
    fun formatOrNull(b: ByteArray): String? {
        if (b.size != 12) return null
        val year = ((b[0].toInt() and 0xFF) shl 8) or (b[1].toInt() and 0xFF)
        val month = b[2].toInt() and 0xFF
        val day = b[3].toInt() and 0xFF
        val hour = b[5].toInt() and 0xFF
        val minute = b[6].toInt() and 0xFF
        val second = b[7].toInt() and 0xFF
        val plausible = (year in 2000..2099 || year == 0xFFFF) &&
            (month in 1..12 || month >= 0xFD) && (day in 1..31 || day >= 0xFD) &&
            (hour in 0..23 || hour == 0xFF) && (minute in 0..59 || minute == 0xFF) &&
            (second in 0..59 || second == 0xFF)
        if (!plausible) return null
        // 0xFD..0xFF in a one-byte field (0xFFFF for the year) mean "not specified".
        fun f(v: Int) = if (v >= 0xFD) "**" else v.toString().padStart(2, '0')
        val y = if (year == 0xFFFF) "****" else year.toString()
        return "$y-${f(month)}-${f(day)} ${f(hour)}:${f(minute)}:${f(second)}"
    }
}

/**
 * [DlmsData.display], with floats rounded to [decimals] places (trailing
 * zeros dropped); null keeps every digit the meter sent.
 */
fun DlmsData.displayNumber(decimals: Int?): String {
    if (decimals == null || this !is DlmsData.Float) return display()
    return display().toBigDecimalOrNull()?.let { roundPlain(it, decimals) } ?: display()
}

/** [value] rounded half-up to at most [decimals] places, as plain text without trailing zeros. */
fun roundPlain(value: java.math.BigDecimal, decimals: Int): String =
    value.setScale(decimals, java.math.RoundingMode.HALF_UP).stripTrailingZeros().toPlainString()

fun ByteArray.toHex(): String = joinToString("") { "%02x".format(it) }

internal fun ByteArray.printableAsciiOrNull(): String? =
    if (isNotEmpty() && all { it in 0x20..0x7E }) String(this, Charsets.US_ASCII) else null
