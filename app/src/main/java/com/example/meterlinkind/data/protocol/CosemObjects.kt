package com.example.meterlinkind.data.protocol

import java.io.ByteArrayOutputStream
import java.time.LocalDateTime

/** One COSEM object of the meter: interface class, version and logical name (OBIS). */
data class CosemObject(
    /** Position in its list: the meter's object list, or `g_ist[]` for [ObjectRegistry]. */
    val index: Int,
    val classId: Int,
    val version: Int,
    val obis: ObisCode,
    val name: String,
    /** Attribute id → access mode for the logged-in client, from the meter's object list; null if unknown. */
    val attributeAccess: Map<Int, Int>? = null,
) {
    /** Unknown access counts as allowed: the meter has the final say. */
    fun canRead(attribute: Int): Boolean = attributeAccess?.let { AccessMode.canRead(it[attribute] ?: AccessMode.NONE) } ?: true

    fun canWrite(attribute: Int): Boolean = attributeAccess?.let { AccessMode.canWrite(it[attribute] ?: AccessMode.NONE) } ?: true

    fun attribute(attribute: Int): CosemAttribute =
        CosemAttribute(classId, obis, attribute, "$name.${InterfaceClass.attributeName(classId, attribute)}")
}

/** Attribute names of the interface classes the app browses (IEC 62056-6-2). */
object InterfaceClass {
    const val DATA = 1
    const val PROFILE_GENERIC = 7
    const val CLOCK = 8
    const val ASSOCIATION_LN = 15

    private val ATTRIBUTES = mapOf(
        DATA to listOf("logical_name", "value"),
        PROFILE_GENERIC to listOf(
            "logical_name", "buffer", "capture_objects", "capture_period",
            "sort_method", "sort_object", "entries_in_use", "profile_entries",
        ),
        CLOCK to listOf(
            "logical_name", "time", "time_zone", "status", "daylight_savings_begin",
            "daylight_savings_end", "daylight_savings_deviation", "daylight_savings_enabled", "clock_base",
        ),
    )

    fun name(classId: Int): String = when (classId) {
        DATA -> "Data"
        3 -> "Register"
        4 -> "Extended Register"
        PROFILE_GENERIC -> "Profile Generic"
        CLOCK -> "Clock"
        ASSOCIATION_LN -> "Association LN"
        else -> "Class $classId"
    }

    fun attributeCount(classId: Int): Int = ATTRIBUTES[classId]?.size ?: 1

    fun attributeName(classId: Int, attribute: Int): String =
        ATTRIBUTES[classId]?.getOrNull(attribute - 1) ?: "attribute $attribute"
}

/** attribute_access_mode of an object_list entry (IEC 62056-6-2, Association LN). */
object AccessMode {
    const val NONE = 0

    fun canRead(mode: Int): Boolean = mode == 1 || mode == 3 || mode == 4 || mode == 6

    fun canWrite(mode: Int): Boolean = mode == 2 || mode == 3 || mode == 5 || mode == 6

    /** Short label for the UI: "R", "W", "R/W" or "none". */
    fun label(mode: Int): String = when {
        canRead(mode) && canWrite(mode) -> "R/W"
        canRead(mode) -> "R"
        canWrite(mode) -> "W"
        else -> "none"
    }
}

// ---------------------------------------------------------------------------
// Selective access and SET
// ---------------------------------------------------------------------------

object SelectiveAccess {
    /**
     * Profile Generic entry_descriptor (selector 2): rows [fromEntry]..[toEntry],
     * 1-based, toEntry 0 = up to the newest; columns likewise, 0 = all
     * (`xdlms.c:cl7_selacc2`).
     */
    fun byEntry(fromEntry: Long, toEntry: Long, fromColumn: Int = 1, toColumn: Int = 0): ByteArray =
        byteArrayOf(0x02, Axdr.TAG_STRUCTURE.toByte(), 0x04) +
            u32(Axdr.TAG_UINT32, fromEntry) + u32(Axdr.TAG_UINT32, toEntry) +
            u16(Axdr.TAG_UINT16, fromColumn) + u16(Axdr.TAG_UINT16, toColumn)

    /**
     * Profile Generic range_descriptor (selector 1): rows whose [restricting]
     * column (one of the capture objects, normally the clock) lies in
     * [from]..[to], all columns (`xdlms.c:cl7_selacc1`). The meter rejects
     * from > to.
     */
    fun byRange(restricting: CaptureObject, from: LocalDateTime, to: LocalDateTime): ByteArray =
        byteArrayOf(0x01, Axdr.TAG_STRUCTURE.toByte(), 0x04) +
            captureObjectDefinition(restricting) +
            ClockValue.encode(from) + ClockValue.encode(to) +
            byteArrayOf(Axdr.TAG_ARRAY.toByte(), 0x00) // selected_values: none = all columns

    private fun captureObjectDefinition(c: CaptureObject): ByteArray =
        byteArrayOf(Axdr.TAG_STRUCTURE.toByte(), 0x04) +
            u16(Axdr.TAG_UINT16, c.classId) +
            byteArrayOf(Axdr.TAG_OCTET_STRING.toByte(), 6) + c.obis.bytes +
            byteArrayOf(Axdr.TAG_INT8.toByte(), c.attribute.toByte()) +
            u16(Axdr.TAG_UINT16, c.dataIndex)

    private fun u32(tag: Int, v: Long) =
        byteArrayOf(tag.toByte(), (v shr 24).toByte(), (v shr 16).toByte(), (v shr 8).toByte(), v.toByte())

    private fun u16(tag: Int, v: Int) = byteArrayOf(tag.toByte(), (v shr 8).toByte(), v.toByte())
}

object CosemSet {
    const val TAG_SET_REQUEST = 0xC1
    const val TAG_SET_RESPONSE = 0xC5

    /** Set-Request-Normal; [value] is one encoded `Data`. */
    fun request(target: CosemAttribute, value: ByteArray, invokeId: Int = Get.INVOKE_ID_AND_PRIORITY): ByteArray =
        byteArrayOf(
            TAG_SET_REQUEST.toByte(), 0x01, invokeId.toByte(),
            (target.classId shr 8).toByte(), target.classId.toByte(),
        ) + target.obis.bytes + byteArrayOf(target.attribute.toByte(), 0x00) + value

    /** Null on success, otherwise why the meter refused. */
    fun parseResponse(apdu: ByteArray): String? {
        val r = Axdr.Reader(apdu)
        when (r.u8()) {
            TAG_SET_RESPONSE -> Unit
            ServiceError.TAG_EXCEPTION_RESPONSE, ServiceError.TAG_CONFIRMED_SERVICE_ERROR ->
                return ServiceError.describe(apdu)
            else -> throw ApduException("Expected SET response, got ${describeApdu(apdu)}")
        }
        if (r.u8() != 0x01) throw ApduException("Unsupported SET response type")
        r.u8() // invoke-id-and-priority
        val result = r.u8()
        return if (result == 0) null else DataAccessResult.describe(result)
    }
}

// ---------------------------------------------------------------------------
// Clock
// ---------------------------------------------------------------------------

object ClockValue {
    /** Deviation "not specified" (IEC 62056-6-2 §4.1.6.1). */
    const val DEVIATION_UNSPECIFIED = 0x8000

    /**
     * Encodes a Clock.time octet-string(12): year, month, day, day of week
     * (1 = Monday), hour, minute, second, hundredths (not specified),
     * deviation and status.
     */
    fun encode(time: LocalDateTime, deviation: Int = DEVIATION_UNSPECIFIED, status: Int = 0x00): ByteArray =
        byteArrayOf(
            Axdr.TAG_OCTET_STRING.toByte(), 12,
            (time.year shr 8).toByte(), time.year.toByte(),
            time.monthValue.toByte(), time.dayOfMonth.toByte(), time.dayOfWeek.value.toByte(),
            time.hour.toByte(), time.minute.toByte(), time.second.toByte(), 0xFF.toByte(),
            (deviation shr 8).toByte(), deviation.toByte(), status.toByte(),
        )

    /** The deviation field of a 12-byte date-time, to reuse when setting the clock. */
    fun deviationOf(dateTime: ByteArray): Int? =
        if (dateTime.size == 12) ((dateTime[9].toInt() and 0xFF) shl 8) or (dateTime[10].toInt() and 0xFF) else null
}

// ---------------------------------------------------------------------------
// Association object list and Profile Generic capture objects
// ---------------------------------------------------------------------------

data class ObjectListEntry(
    val classId: Int,
    val version: Int,
    val obis: ObisCode,
    /** Attribute id → access mode; attributes the meter doesn't list have no access. Null if unknown. */
    val attributeAccess: Map<Int, Int>? = null,
)

/** One column of a Profile Generic buffer (capture_object_definition). */
data class CaptureObject(val classId: Int, val obis: ObisCode, val attribute: Int, val dataIndex: Int) {
    /** Column title: the registry name if the meter's table has the object, else the OBIS code. */
    val label: String
        get() = ObjectRegistry.find(obis)?.name ?: "$obis"
}

object CosemStructures {
    /** Association LN attribute 2: array of {class_id, version, logical_name, access_rights}. */
    fun objectList(data: DlmsData): List<ObjectListEntry> =
        (data as? DlmsData.Array ?: throw ApduException("object_list is not an array")).items.map { item ->
            val fields = (item as? DlmsData.Structure)?.items ?: throw ApduException("Bad object_list element")
            ObjectListEntry(
                classId = fields.int(0),
                version = fields.int(1),
                obis = ObisCode.of((fields[2] as DlmsData.OctetString).bytes),
                attributeAccess = fields.getOrNull(3)?.let(::attributeAccess),
            )
        }

    /** access_rights: {attribute_access: array of {attribute_id, access_mode, access_selectors}, method_access}. */
    private fun attributeAccess(rights: DlmsData): Map<Int, Int>? {
        val attributes = ((rights as? DlmsData.Structure)?.items?.firstOrNull() as? DlmsData.Array)?.items ?: return null
        return attributes.mapNotNull { item ->
            val fields = (item as? DlmsData.Structure)?.items ?: return@mapNotNull null
            runCatching { fields.int(0) to fields.int(1) }.getOrNull()
        }.toMap()
    }

    /** Profile Generic attribute 3: array of {class_id, logical_name, attribute_index, data_index}. */
    fun captureObjects(data: DlmsData): List<CaptureObject> =
        (data as? DlmsData.Array ?: throw ApduException("capture_objects is not an array")).items.map { item ->
            val fields = (item as? DlmsData.Structure)?.items ?: throw ApduException("Bad capture object")
            CaptureObject(
                classId = fields.int(0),
                obis = ObisCode.of((fields[1] as DlmsData.OctetString).bytes),
                attribute = fields.int(2),
                dataIndex = fields.int(3),
            )
        }

    private fun List<DlmsData>.int(i: Int): Int = when (val v = get(i)) {
        is DlmsData.Integer -> v.value.toInt()
        is DlmsData.Enum -> v.value
        else -> throw ApduException("Expected an integer, got $v")
    }
}

// ---------------------------------------------------------------------------
// General Block Transfer
// ---------------------------------------------------------------------------

/**
 * General-Block-Transfer APDU (IEC 62056-5-3 §9.3.10). The FT5AB sends large
 * ciphered replies this way, streamed without acknowledgement
 * (`hrdtushin.c:wrapCtrl`): the block data of all blocks together is one
 * APDU, typically general-glo-ciphering.
 */
data class GbtBlock(
    val last: Boolean,
    val streaming: Boolean,
    val window: Int,
    val blockNumber: Int,
    val blockNumberAck: Int,
    val data: ByteArray,
) {
    companion object {
        const val TAG = 0xE0

        fun parse(apdu: ByteArray): GbtBlock {
            val r = Axdr.Reader(apdu)
            if (r.u8() != TAG) throw ApduException("Expected GBT, got ${describeApdu(apdu)}")
            val control = r.u8()
            return GbtBlock(
                last = control and 0x80 != 0,
                streaming = control and 0x40 != 0,
                window = control and 0x3F,
                blockNumber = r.uint(2).toInt(),
                blockNumberAck = r.uint(2).toInt(),
                data = r.bytes(r.length()),
            )
        }

        /** Our acknowledgement asking for the next window when the meter isn't streaming. */
        fun ack(ourBlockNumber: Int, receivedBlockNumber: Int, window: Int = 1): ByteArray =
            byteArrayOf(
                TAG.toByte(), window.toByte(),
                (ourBlockNumber shr 8).toByte(), ourBlockNumber.toByte(),
                (receivedBlockNumber shr 8).toByte(), receivedBlockNumber.toByte(),
                0x00,
            )
    }
}

/** Collects GBT block data in order until the last block. */
class GbtAssembler {
    private val data = ByteArrayOutputStream()
    var lastBlockNumber = 0
        private set

    /** Adds one block; returns the whole APDU after the last block, else null. */
    fun add(block: GbtBlock): ByteArray? {
        if (block.blockNumber != lastBlockNumber + 1) {
            throw ApduException("GBT block ${block.blockNumber} out of order (expected ${lastBlockNumber + 1})")
        }
        lastBlockNumber = block.blockNumber
        data.write(block.data)
        return if (block.last) data.toByteArray() else null
    }
}
