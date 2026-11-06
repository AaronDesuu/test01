package com.example.meterlinkind.data.protocol

import java.math.BigDecimal
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter
import java.time.format.DateTimeParseException

/**
 * How an attribute's value is edited as text: what the field starts with,
 * what kind of input it takes, and how the text becomes the new value. The
 * new value keeps the type of the one read from the meter.
 */
class ValueEditor(
    /** Shown above the field, e.g. "Number (long-unsigned)". */
    val label: String,
    val initial: String,
    val kind: Kind,
    /** Help under the field, e.g. the allowed range. */
    val hint: String? = null,
    private val parser: (String) -> DlmsData,
) {
    enum class Kind { NUMBER, DECIMAL, TEXT, HEX, BOOLEAN, DATE_TIME, BITS }

    /** The new value, or why the text isn't one. */
    fun parse(text: String): Result<DlmsData> = runCatching { parser(text.trim()) }

    companion object {
        private val DATE_TIME = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss")

        /**
         * The editor for [current]. A Register value with a known [scaler] is
         * edited in its unit (239.12 for 23912 × 10⁻² V).
         */
        fun forValue(current: DlmsData, scaler: ScalerUnit? = null): ValueEditor = when (current) {
            is DlmsData.Bool -> ValueEditor("On / off (boolean)", current.value.toString(), Kind.BOOLEAN) {
                when (it.lowercase()) {
                    "true" -> DlmsData.Bool(true)
                    "false" -> DlmsData.Bool(false)
                    else -> error("Choose true or false")
                }
            }
            is DlmsData.Integer -> integerEditor(current, scaler)
            is DlmsData.Enum -> ValueEditor("Number (enum)", current.value.toString(), Kind.NUMBER, "0 to 255") {
                val v = it.toIntOrNull() ?: error("Enter a whole number")
                require(v in 0..255) { "Must be 0 to 255" }
                DlmsData.Enum(v)
            }
            is DlmsData.Float -> ValueEditor("Number (${Axdr.typeName(current)})", current.display(), Kind.DECIMAL) {
                DlmsData.Float(it.toDoubleOrNull() ?: error("Enter a number"), current.singlePrecision)
            }
            is DlmsData.OctetString -> octetEditor(current.bytes) { DlmsData.OctetString(it) }
            is DlmsData.DateTime -> if (current.bytes.size == 12) {
                octetEditor(current.bytes) { DlmsData.DateTime(it) }
            } else rawEditor(current)
            is DlmsData.VisibleString -> ValueEditor("Text (visible-string)", current.value, Kind.TEXT) {
                require(it.all { c -> c.code in 0x20..0x7E }) { "Only printable ASCII characters" }
                DlmsData.VisibleString(it)
            }
            is DlmsData.Utf8String -> ValueEditor("Text (utf8-string)", current.value, Kind.TEXT) { DlmsData.Utf8String(it) }
            is DlmsData.BitString -> ValueEditor(
                "Bits (bit-string)", bitsOf(current), Kind.BITS, "${current.bitCount} bits, 0 or 1"
            ) {
                require(it.isNotEmpty() && it.all { c -> c == '0' || c == '1' }) { "Only 0 and 1" }
                DlmsData.BitString(it.length, packBits(it))
            }
            else -> rawEditor(current)
        }

        private fun integerEditor(current: DlmsData.Integer, scaler: ScalerUnit?): ValueEditor {
            val type = Axdr.typeName(current)
            val range = Axdr.integerRange(current.tag)
            if (scaler != null && scaler.scaler != 0) {
                val unit = scaler.symbol.ifEmpty { "no unit" }
                val start = BigDecimal.valueOf(current.value).scaleByPowerOfTen(scaler.scaler).stripTrailingZeros().toPlainString()
                val step = BigDecimal.ONE.scaleByPowerOfTen(scaler.scaler).stripTrailingZeros().toPlainString()
                return ValueEditor("Value in $unit ($type × 10^${scaler.scaler})", start, Kind.DECIMAL, "In steps of $step") {
                    val scaled = it.toBigDecimalOrNull() ?: error("Enter a number")
                    val raw = scaled.scaleByPowerOfTen(-scaler.scaler)
                    val whole = runCatching { raw.longValueExact() }.getOrNull()
                        ?: error("Must be a multiple of $step")
                    require(whole in range) { "Out of range for $type" }
                    DlmsData.Integer(current.tag, whole)
                }
            }
            return ValueEditor("Number ($type)", current.display(), Kind.NUMBER, "${range.first} to ${range.last}") {
                val v = it.toLongOrNull() ?: error("Enter a whole number")
                require(v in range) { "Must be ${range.first} to ${range.last}" }
                DlmsData.Integer(current.tag, v)
            }
        }

        /** A date-time when the bytes are one, readable text when printable, hex otherwise. */
        private fun octetEditor(bytes: ByteArray, make: (ByteArray) -> DlmsData): ValueEditor {
            CosemDateTime.formatOrNull(bytes)?.let { shown ->
                return ValueEditor("Date and time", shown, Kind.DATE_TIME, "yyyy-MM-dd HH:mm:ss") {
                    val time = try {
                        LocalDateTime.parse(it, DATE_TIME)
                    } catch (e: DateTimeParseException) {
                        error("Use yyyy-MM-dd HH:mm:ss")
                    }
                    // Keep the deviation and status the meter sent.
                    val deviation = ClockValue.deviationOf(bytes) ?: ClockValue.DEVIATION_UNSPECIFIED
                    val encoded = ClockValue.encode(time, deviation, bytes[11].toInt() and 0xFF)
                    make(encoded.copyOfRange(2, encoded.size))
                }
            }
            bytes.printableAsciiOrNull()?.let { text ->
                return ValueEditor("Text (octet-string)", text, Kind.TEXT) {
                    require(it.all { c -> c.code in 0x20..0x7E }) { "Only printable ASCII characters" }
                    make(it.toByteArray(Charsets.US_ASCII))
                }
            }
            return ValueEditor("Bytes in hex (octet-string)", bytes.toHex().uppercase(), Kind.HEX, "${bytes.size} bytes now") {
                make(hexToBytes(it))
            }
        }

        /** A logical name (6-byte octet-string) as an OBIS code, 0.0.10.0.100.255. */
        fun forObis(bytes: ByteArray): ValueEditor = ValueEditor(
            "OBIS code (octet-string)",
            bytes.joinToString(".") { (it.toInt() and 0xFF).toString() },
            Kind.TEXT,
            "Six numbers 0 to 255, like 0.0.10.0.100.255",
        ) { text ->
            val parts = text.split('.')
            require(parts.size == 6) { "Six numbers separated by dots" }
            DlmsData.OctetString(ByteArray(6) { i ->
                val n = parts[i].trim().toIntOrNull()
                require(n != null && n in 0..255) { "Each number must be 0 to 255" }
                n.toByte()
            })
        }

        /** For a write-only attribute: its type is unknown, so the whole value is typed as A-XDR hex. */
        fun forUnknown(): ValueEditor = ValueEditor(
            "Encoded value in hex",
            "",
            Kind.HEX,
            "Can't be read, so type the whole A-XDR value, tag included (09 10 … for 16 bytes)",
        ) { Axdr.decodeWhole(hexToBytes(it)) }

        /** Structures, arrays and the rest: the whole value as A-XDR hex. */
        private fun rawEditor(current: DlmsData): ValueEditor = ValueEditor(
            "Encoded value in hex (${Axdr.typeName(current)})",
            Axdr.encode(current).toHex().uppercase(),
            Kind.HEX,
            "The whole A-XDR value, tag included",
        ) { Axdr.decodeWhole(hexToBytes(it)) }

        fun hexToBytes(text: String): ByteArray {
            val hex = text.filterNot { it.isWhitespace() }
            require(hex.isNotEmpty()) { "Enter some hex" }
            require(hex.length % 2 == 0) { "Hex needs an even number of digits" }
            require(hex.all { it in '0'..'9' || it.lowercaseChar() in 'a'..'f' }) { "Only 0-9 and A-F" }
            return ByteArray(hex.length / 2) { hex.substring(2 * it, 2 * it + 2).toInt(16).toByte() }
        }

        private fun bitsOf(bits: DlmsData.BitString): String = buildString {
            for (i in 0 until bits.bitCount) {
                val byte = bits.bytes[i / 8].toInt()
                append(if (byte and (0x80 shr (i % 8)) != 0) '1' else '0')
            }
        }

        private fun packBits(bits: String): ByteArray {
            val out = ByteArray((bits.length + 7) / 8)
            bits.forEachIndexed { i, c -> if (c == '1') out[i / 8] = (out[i / 8].toInt() or (0x80 shr (i % 8))).toByte() }
            return out
        }
    }
}
