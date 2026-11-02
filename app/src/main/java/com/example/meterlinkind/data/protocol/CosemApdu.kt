package com.example.meterlinkind.data.protocol

/** OBIS code, e.g. `ObisCode.parse("0.0.96.1.0.255")`. */
@JvmInline
value class ObisCode private constructor(private val packed: Long) {
    val bytes: ByteArray get() = ByteArray(6) { i -> (packed shr (8 * (5 - i))).toByte() }

    override fun toString(): String = bytes.joinToString(".") { (it.toInt() and 0xFF).toString() }

    companion object {
        fun parse(text: String): ObisCode {
            val parts = text.split('.').map { it.toInt() }
            require(parts.size == 6 && parts.all { it in 0..255 }) { "Bad OBIS code: $text" }
            return of(parts.map { it.toByte() }.toByteArray())
        }

        fun of(bytes: ByteArray): ObisCode {
            require(bytes.size == 6)
            return ObisCode(bytes.fold(0L) { acc, b -> (acc shl 8) or (b.toLong() and 0xFF) })
        }
    }
}

/** One attribute of one COSEM object: what a GET or SET addresses. */
data class CosemAttribute(val classId: Int, val obis: ObisCode, val attribute: Int, val name: String)

/**
 * Objects reachable by the Public (PC) client, as listed by Gurux DLMS Director
 * on an FT5AB meter. The full table lives in `xdlms.c g_ist[]`.
 */
object PublicObjects {
    val LOGICAL_DEVICE_NAME = CosemAttribute(1, ObisCode.parse("0.0.42.0.0.255"), 2, "COSEM Logical Device Name")
    val INVOCATION_COUNTER_US = CosemAttribute(1, ObisCode.parse("0.0.43.1.3.255"), 2, "Invocation Counter #3 (US)")
    val INVOCATION_COUNTER_PUSH = CosemAttribute(1, ObisCode.parse("0.0.43.1.4.255"), 2, "Invocation Counter #4 (Push)")
    val INVOCATION_COUNTER_FW = CosemAttribute(1, ObisCode.parse("0.0.43.1.5.255"), 2, "Invocation Counter #5 (FW)")
    val SERIAL_NUMBER = CosemAttribute(1, ObisCode.parse("0.0.96.1.0.255"), 2, "Meter Serial Number")
    val CLOCK = CosemAttribute(8, ObisCode.parse("0.0.1.0.0.255"), 2, "Clock")
    val CURRENT_ASSOCIATION = CosemAttribute(15, ObisCode.parse("0.0.40.0.0.255"), 2, "Current Association")
    val PC_ASSOCIATION = CosemAttribute(15, ObisCode.parse("0.0.40.0.1.255"), 2, "PC Association")
}

/** Authentication mechanism ids (`xdlms.h MECHA_…`), the last arc of the mechanism name. */
enum class Mechanism(val id: Int) {
    NONE(0),
    /** Password in the AARQ. */
    LLS(1),
    /** AES-128 of each side's challenge with the shared secret, then a reply_to_HLS_authentication. */
    HLS(2),
}

/**
 * IS 15959 client SAPs on the FT5AB. The meter turns the wrapper source port into
 * `(port << 1) | 1` and looks it up in `dlms.c USERS[]`, which also fixes the
 * security control each one uses (`sys.sc`).
 */
enum class ClientProfile(
    val clientPort: Int,
    val label: String,
    val mechanism: Mechanism,
    /** 0 = no ciphering; otherwise [SecuritySuite0] SC bits. */
    val securityControl: Int,
) {
    PUBLIC(0x10, "Public (PC)", Mechanism.NONE, 0),
    /** `USERS[]` maddr 0x41 (COSEM_POWER): LLS, encrypted. */
    METER_READER(0x20, "Meter Reader (MR)", Mechanism.LLS, SecuritySuite0.SC_ENCRYPTED),
    /** `USERS[]` maddr 0x61 (COSEM_ADMIN): HLS, encrypted and authenticated. */
    UTILITY_SETTINGS(
        0x30, "Utility Settings (US)", Mechanism.HLS,
        SecuritySuite0.SC_ENCRYPTED or SecuritySuite0.SC_AUTHENTICATED
    ),
}

/** Conformance bits (IEC 62056-5-3 §7.3.1); bit 0 is the MSB of the first byte. */
object Conformance {
    const val GENERAL_PROTECTION = 1 shl (23 - 1)
    const val GENERAL_BLOCK_TRANSFER = 1 shl (23 - 2)
    const val DELTA_VALUE_ENCODING = 1 shl (23 - 6)
    const val BLOCK_TRANSFER_WITH_GET = 1 shl (23 - 11)
    const val BLOCK_TRANSFER_WITH_SET = 1 shl (23 - 12)
    const val BLOCK_TRANSFER_WITH_ACTION = 1 shl (23 - 13)
    const val MULTIPLE_REFERENCES = 1 shl (23 - 14)
    const val ACCESS = 1 shl (23 - 17)
    const val GET = 1 shl (23 - 19)
    const val SET = 1 shl (23 - 20)
    const val SELECTIVE_ACCESS = 1 shl (23 - 21)
    const val ACTION = 1 shl (23 - 23)

    /**
     * What we propose: `62 1E 5D`, the same as Gurux, whose wrapper AARQ the
     * meter accepts. The meter ANDs this with its per-client table
     * (`xdlms.c CONFORMANCE[]`, Public is GET only) and rejects any SN
     * read/write bits, so none are set here.
     */
    const val PROPOSED = GENERAL_PROTECTION or GENERAL_BLOCK_TRANSFER or DELTA_VALUE_ENCODING or
        BLOCK_TRANSFER_WITH_GET or BLOCK_TRANSFER_WITH_SET or BLOCK_TRANSFER_WITH_ACTION or
        MULTIPLE_REFERENCES or ACCESS or GET or SET or SELECTIVE_ACCESS or ACTION

    fun describe(bits: Int): String = listOfNotNull(
        "GBT".takeIf { bits and GENERAL_BLOCK_TRANSFER != 0 },
        "block-get".takeIf { bits and BLOCK_TRANSFER_WITH_GET != 0 },
        "block-set".takeIf { bits and BLOCK_TRANSFER_WITH_SET != 0 },
        "block-action".takeIf { bits and BLOCK_TRANSFER_WITH_ACTION != 0 },
        "multi-ref".takeIf { bits and MULTIPLE_REFERENCES != 0 },
        "get".takeIf { bits and GET != 0 },
        "set".takeIf { bits and SET != 0 },
        "selective".takeIf { bits and SELECTIVE_ACCESS != 0 },
        "action".takeIf { bits and ACTION != 0 },
    ).joinToString(",").ifEmpty { "none" }
}

class ApduException(message: String) : Exception(message)

// ---------------------------------------------------------------------------
// Association (ACSE)
// ---------------------------------------------------------------------------

data class AssociationResult(
    val accepted: Boolean,
    /** 0 accepted, 1 rejected-permanent, 2 rejected-transient. */
    val result: Int,
    /** Source of the diagnostic: 1 = acse-service-user, 2 = acse-service-provider. */
    val diagnosticSource: Int,
    val diagnostic: Int,
    val negotiatedConformance: Int,
    val serverMaxReceivePdu: Int,
    val dlmsVersion: Int,
    /** Set when the meter answered the InitiateRequest with a confirmed-service-error. */
    val initiateError: String? = null,
    /** responding-AP-title (A4): the meter's system title, the IV prefix of its ciphered replies. */
    val serverSystemTitle: ByteArray? = null,
    /** responding-authentication-value (AA): the meter's HLS challenge (StoC). */
    val serverChallenge: ByteArray? = null,
) {
    override fun toString(): String =
        "AssociationResult(accepted=$accepted, result=$result, diagnosticSource=$diagnosticSource, " +
            "diagnostic=$diagnostic, negotiatedConformance=$negotiatedConformance, " +
            "serverMaxReceivePdu=$serverMaxReceivePdu, dlmsVersion=$dlmsVersion, initiateError=$initiateError, " +
            "serverSystemTitle=${serverSystemTitle?.toHex()}, serverChallenge=${serverChallenge?.toHex()})"

    val reason: String
        get() = initiateError ?: when (diagnosticSource) {
            1 -> when (diagnostic) {
                0 -> "OK"
                1 -> "no reason given"
                2 -> "application context name not supported"
                11 -> "authentication mechanism name not recognised"
                12 -> "authentication mechanism name required"
                13 -> "authentication failure"
                14 -> "authentication required"
                else -> "service-user diagnostic $diagnostic"
            }
            2 -> if (diagnostic == 1) "no reason given" else "service-provider diagnostic $diagnostic"
            else -> "diagnostic $diagnostic"
        }
}

object Association {
    const val TAG_AARQ = 0x60
    const val TAG_AARE = 0x61
    const val TAG_RLRQ = 0x62
    const val TAG_RLRE = 0x63

    /** Release reason normal (`RL_NORMAL`); the meter answers not-finished (1) when it refuses. */
    private const val RELEASE_NORMAL = 0

    private val LN_NO_CIPHER = byteArrayOf(0x60, 0x85.toByte(), 0x74, 0x05, 0x08, 0x01, 0x01)
    private val LN_CIPHER = byteArrayOf(0x60, 0x85.toByte(), 0x74, 0x05, 0x08, 0x01, 0x03)
    private val MECHANISM_NAME_PREFIX = byteArrayOf(0x60, 0x85.toByte(), 0x74, 0x05, 0x08, 0x02)

    /**
     * Our max receive PDU size, `FFFF` like Gurux. The meter caps it at its own
     * (0x02E2 on the FT5AB), and `WrapperAssembler` handles any size.
     */
    const val CLIENT_MAX_RECEIVE_PDU = 0xFFFF

    /** xDLMS InitiateRequest, the user-information of every AARQ (ciphered for MR and US). */
    fun initiateRequest(
        conformance: Int = Conformance.PROPOSED,
        maxReceivePdu: Int = CLIENT_MAX_RECEIVE_PDU,
    ): ByteArray = byteArrayOf(
        0x01,                           // InitiateRequest
        0x00,                           // dedicated-key: absent
        0x00,                           // response-allowed: default (TRUE)
        0x00,                           // proposed-quality-of-service: absent
        0x06,                           // proposed-dlms-version-number
        0x5F, 0x1F, 0x04, 0x00,         // [APPLICATION 31] Conformance, 4 bytes, 0 unused bits
        (conformance shr 16).toByte(), (conformance shr 8).toByte(), conformance.toByte(),
        (maxReceivePdu shr 8).toByte(), maxReceivePdu.toByte(),
    )

    /**
     * AARQ for the Public client: application-context-name (A1) and
     * user-information (BE) only. For Public the meter rejects any other tag,
     * including a mechanism name of "none" (`acse.c:AARE`, `client0[]`).
     */
    fun publicRequest(
        conformance: Int = Conformance.PROPOSED,
        maxReceivePdu: Int = CLIENT_MAX_RECEIVE_PDU,
    ): ByteArray {
        val body = Ber.tlv(0xA1, Ber.tlv(0x06, LN_NO_CIPHER)) +
            Ber.tlv(0xBE, Ber.tlv(0x04, initiateRequest(conformance, maxReceivePdu)))
        return Ber.tlv(TAG_AARQ, body)
    }

    /**
     * AARQ for MR and US. The meter wants all six of A1 (LN with ciphering), A6
     * calling-AP-title, 8A acse-requirements, 8B mechanism name, AC calling
     * authentication value and BE, in that order (`acse.c:AARE`, `client2[]`).
     *
     * [authenticationValue] is the LLS password, or our HLS challenge (CtoS).
     * [cipheredInitiate] is the InitiateRequest after [SecureChannel.protect].
     */
    fun cipheredRequest(
        clientSystemTitle: ByteArray,
        mechanism: Mechanism,
        authenticationValue: ByteArray,
        cipheredInitiate: ByteArray,
    ): ByteArray {
        val body = Ber.tlv(0xA1, Ber.tlv(0x06, LN_CIPHER)) +
            Ber.tlv(0xA6, Ber.tlv(0x04, clientSystemTitle)) +
            Ber.tlv(0x8A, byteArrayOf(0x07, 0x80.toByte())) + // authentication functional unit
            Ber.tlv(0x8B, MECHANISM_NAME_PREFIX + mechanism.id.toByte()) +
            Ber.tlv(0xAC, Ber.tlv(0x80, authenticationValue)) +
            Ber.tlv(0xBE, Ber.tlv(0x04, cipheredInitiate))
        return Ber.tlv(TAG_AARQ, body)
    }

    /**
     * RLRQ with reason normal. The meter refuses a bare `62 00`: it wants
     * exactly the reason element `80 01 00` (`acse.c:request_release_reason`).
     */
    fun releaseRequest(): ByteArray = Ber.tlv(TAG_RLRQ, Ber.tlv(0x80, byteArrayOf(RELEASE_NORMAL.toByte())))

    /** True if the RLRE has reason normal, i.e. the meter dropped the association. */
    fun parseReleaseResponse(apdu: ByteArray): Boolean {
        if (apdu.isEmpty() || (apdu[0].toInt() and 0xFF) != TAG_RLRE) {
            throw ApduException("Expected RLRE (0x63), got ${describeApdu(apdu)}")
        }
        val reason = Ber.parse(Ber.parse(apdu).single().value).firstOrNull { it.tag == 0x80 }
            ?: throw ApduException("RLRE has no reason")
        return reason.intValue() == RELEASE_NORMAL
    }

    /**
     * Parses an AARE. A ciphered InitiateResponse (glo-initiate-response) is
     * deciphered with [channel], which also learns the meter's system title.
     */
    fun parseResponse(apdu: ByteArray, channel: SecureChannel? = null): AssociationResult {
        if (apdu.isEmpty()) throw ApduException("Empty association response")
        if ((apdu[0].toInt() and 0xFF) != TAG_AARE) {
            throw ApduException("Expected AARE (0x61), got ${describeApdu(apdu)}")
        }
        val outer = Ber.parse(apdu).single()
        var result = -1
        var diagSource = 0
        var diag = 0
        var conformance = 0
        var maxPdu = 0
        var version = 0
        var initiateError: String? = null
        var serverSystemTitle: ByteArray? = null
        var serverChallenge: ByteArray? = null

        for (el in Ber.parse(outer.value)) {
            when (el.tag) {
                0xA2 -> result = Ber.parse(el.value).single().intValue()
                0xA3 -> Ber.parse(el.value).single().let { choice ->
                    diagSource = choice.tag and 0x1F
                    diag = Ber.parse(choice.value).single().intValue()
                }
                0xA4 -> serverSystemTitle = Ber.parse(el.value).single().value.also {
                    channel?.serverSystemTitle = it
                }
                0xAA -> serverChallenge = Ber.parse(el.value).single().value
                0xBE -> {
                    var info = Ber.parse(el.value).single().value // OCTET STRING contents
                    if (info.firstOrNull()?.toInt()?.and(0xFF) == 0x28) {
                        info = (channel ?: throw ApduException("Ciphered AARE but no keys")).unprotect(info)
                    }
                    when (info.firstOrNull()?.toInt()?.and(0xFF)) {
                        0x08 -> { // InitiateResponse
                            val r = Axdr.Reader(info, 1)
                            if (r.u8() != 0) r.u8() // negotiated-quality-of-service present
                            version = r.u8()
                            if (r.u8() != 0x5F || r.u8() != 0x1F || r.u8() != 0x04) {
                                throw ApduException("Bad conformance tag in InitiateResponse")
                            }
                            r.u8() // unused bits
                            conformance = r.uint(3).toInt()
                            maxPdu = r.uint(2).toInt()
                        }
                        0x0E -> initiateError = "InitiateRequest refused: " +
                            ServiceError.describe(info.copyOfRange(1, info.size))
                    }
                }
            }
        }
        if (result < 0) throw ApduException("AARE has no result field")
        return AssociationResult(
            accepted = result == 0 && initiateError == null,
            result = result,
            diagnosticSource = diagSource,
            diagnostic = diag,
            negotiatedConformance = conformance,
            serverMaxReceivePdu = maxPdu,
            dlmsVersion = version,
            initiateError = initiateError,
            serverSystemTitle = serverSystemTitle,
            serverChallenge = serverChallenge,
        )
    }

    /**
     * HLS pass 3: reply_to_HLS_authentication (current association, method 1)
     * with f(StoC) = AES-128(secret, StoC). The meter answers with f(CtoS).
     */
    fun hlsReplyRequest(secret: ByteArray, serverChallenge: ByteArray): ByteArray =
        Action.request(
            CURRENT_ASSOCIATION_HLS_REPLY,
            byteArrayOf(Axdr.TAG_OCTET_STRING.toByte(), 16) + SecuritySuite0.aesEcb(secret, serverChallenge)
        )

    /** HLS pass 4: true if the meter's f(CtoS) proves it knows [secret]. */
    fun hlsVerify(secret: ByteArray, clientChallenge: ByteArray, response: ActionResult): Boolean {
        val data = (response as? ActionResult.Success)?.data as? DlmsData.OctetString ?: return false
        return data.bytes.contentEquals(SecuritySuite0.aesEcb(secret, clientChallenge))
    }

    private val CURRENT_ASSOCIATION_HLS_REPLY =
        CosemMethod(15, ObisCode.parse("0.0.40.0.0.255"), 1, "reply_to_HLS_authentication")
}

// ---------------------------------------------------------------------------
// ACTION service
// ---------------------------------------------------------------------------

/** One method of one COSEM object: what an ACTION invokes. */
data class CosemMethod(val classId: Int, val obis: ObisCode, val method: Int, val name: String)

sealed class ActionResult {
    /** [data] is the return parameter, if the meter sent one. */
    data class Success(val data: DlmsData?) : ActionResult()
    data class Failure(val code: Int) : ActionResult() {
        val reason: String get() = DataAccessResult.describe(code)
    }
    data class ServiceFailure(val message: String) : ActionResult()
}

object Action {
    const val TAG_ACTION_REQUEST = 0xC3
    const val TAG_ACTION_RESPONSE = 0xC7

    /** Action-Request-Normal; [parameter] is one encoded `Data`, or null for none. */
    fun request(
        target: CosemMethod,
        parameter: ByteArray?,
        invokeId: Int = Get.INVOKE_ID_AND_PRIORITY,
    ): ByteArray =
        byteArrayOf(
            TAG_ACTION_REQUEST.toByte(), 0x01, invokeId.toByte(),
            (target.classId shr 8).toByte(), target.classId.toByte(),
        ) + target.obis.bytes + target.method.toByte() +
            (if (parameter == null) byteArrayOf(0x00) else byteArrayOf(0x01) + parameter)

    fun parseResponse(apdu: ByteArray): ActionResult {
        val r = Axdr.Reader(apdu)
        when (r.u8()) {
            TAG_ACTION_RESPONSE -> Unit
            ServiceError.TAG_EXCEPTION_RESPONSE, ServiceError.TAG_CONFIRMED_SERVICE_ERROR ->
                return ActionResult.ServiceFailure(ServiceError.describe(apdu))
            else -> throw ApduException("Expected ACTION response, got ${describeApdu(apdu)}")
        }
        val type = r.u8()
        if (type != 0x01) throw ApduException("Unsupported ACTION response type $type")
        r.u8() // invoke-id-and-priority
        val result = r.u8()
        if (result != 0) return ActionResult.Failure(result)
        if (r.remaining == 0 || r.u8() == 0) return ActionResult.Success(null)
        // Get-Data-Result: CHOICE data [0] | data-access-result [1]
        return when (r.u8()) {
            0x00 -> ActionResult.Success(r.readData())
            else -> ActionResult.Failure(r.u8())
        }
    }
}

// ---------------------------------------------------------------------------
// GET service
// ---------------------------------------------------------------------------

sealed class GetResult {
    data class Data(val data: DlmsData) : GetResult()
    data class AccessError(val code: Int) : GetResult() {
        val reason: String get() = DataAccessResult.describe(code)
    }
    /** Get-Response-With-Datablock: [raw] is a slice of the encoded `Data`. */
    data class Block(val blockNumber: Long, val last: Boolean, val raw: ByteArray) : GetResult()
    data class BlockError(val blockNumber: Long, val code: Int) : GetResult()
    /** Exception-Response or Confirmed-Service-Error instead of a GET response. */
    data class ServiceFailure(val message: String) : GetResult()
}

object Get {
    const val TAG_GET_REQUEST = 0xC0
    const val TAG_GET_RESPONSE = 0xC4

    /** invoke-id 1, confirmed, high priority, as Gurux sends it. The meter echoes it. */
    const val INVOKE_ID_AND_PRIORITY = 0xC1

    /** [access] is a selector plus its parameters (e.g. [SelectiveAccess.byEntry]), or null for none. */
    fun request(target: CosemAttribute, access: ByteArray? = null, invokeId: Int = INVOKE_ID_AND_PRIORITY): ByteArray =
        byteArrayOf(
            TAG_GET_REQUEST.toByte(), 0x01, invokeId.toByte(),
            (target.classId shr 8).toByte(), target.classId.toByte(),
        ) + target.obis.bytes + target.attribute.toByte() +
            (if (access == null) byteArrayOf(0x00) else byteArrayOf(0x01) + access)

    fun nextBlock(blockNumber: Long, invokeId: Int = INVOKE_ID_AND_PRIORITY): ByteArray =
        byteArrayOf(
            TAG_GET_REQUEST.toByte(), 0x02, invokeId.toByte(),
            (blockNumber shr 24).toByte(), (blockNumber shr 16).toByte(),
            (blockNumber shr 8).toByte(), blockNumber.toByte(),
        )

    fun parseResponse(apdu: ByteArray): GetResult {
        val r = Axdr.Reader(apdu)
        when (val tag = r.u8()) {
            TAG_GET_RESPONSE -> Unit
            ServiceError.TAG_EXCEPTION_RESPONSE, ServiceError.TAG_CONFIRMED_SERVICE_ERROR ->
                return GetResult.ServiceFailure(ServiceError.describe(apdu))
            else -> throw ApduException("Expected GET response, got ${describeApdu(apdu)}")
        }
        val type = r.u8()
        r.u8() // invoke-id-and-priority
        return when (type) {
            0x01 -> when (r.u8()) { // Get-Response-Normal: CHOICE data | data-access-result
                0x00 -> GetResult.Data(r.readData())
                else -> GetResult.AccessError(r.u8())
            }
            0x02 -> { // Get-Response-With-Datablock
                val last = r.u8() != 0
                val blockNo = r.uint(4)
                when (r.u8()) {
                    0x00 -> GetResult.Block(blockNo, last, r.bytes(r.length()))
                    else -> GetResult.BlockError(blockNo, r.u8())
                }
            }
            else -> throw ApduException("Unsupported GET response type $type")
        }
    }
}

object DataAccessResult {
    fun describe(code: Int): String = when (code) {
        0 -> "success"
        1 -> "hardware fault"
        2 -> "temporary failure"
        3 -> "read/write denied"
        4 -> "object undefined"
        9 -> "object class inconsistent"
        11 -> "object unavailable"
        12 -> "type unmatched"
        13 -> "scope of access violated"
        14 -> "data block unavailable"
        15 -> "long get aborted"
        16 -> "no long get in progress"
        17 -> "long set aborted"
        18 -> "no long set in progress"
        19 -> "data block number invalid"
        250 -> "other reason"
        else -> "access result $code"
    } + " ($code)"
}

object ServiceError {
    const val TAG_CONFIRMED_SERVICE_ERROR = 0x0E
    const val TAG_EXCEPTION_RESPONSE = 0xD8

    fun describe(apdu: ByteArray): String {
        if (apdu.isEmpty()) return "empty"
        return when (apdu[0].toInt() and 0xFF) {
            TAG_EXCEPTION_RESPONSE -> {
                val state = apdu.getOrNull(1)?.toInt()?.and(0xFF)
                val service = apdu.getOrNull(2)?.toInt()?.and(0xFF)
                val stateText = when (state) { 1 -> "service not allowed"; 2 -> "service unknown"; else -> "state $state" }
                val serviceText = when (service) {
                    1 -> "operation not possible"; 2 -> "service not supported"; 3 -> "other reason"
                    4 -> "PDU too long"; 5 -> "deciphering error"; 6 -> "invocation counter error"
                    else -> "error $service"
                }
                "exception: $stateText, $serviceText"
            }
            TAG_CONFIRMED_SERVICE_ERROR -> "confirmed service error ${apdu.drop(1).toByteArray().toHex()}"
            else -> apdu.toHex()
        }
    }
}

internal fun describeApdu(apdu: ByteArray): String =
    if (apdu.isEmpty()) "empty APDU" else "0x%02x (%s)".format(apdu[0].toInt() and 0xFF, apdu.toHex())

// ---------------------------------------------------------------------------
// Minimal BER (only what ACSE needs)
// ---------------------------------------------------------------------------

object Ber {
    class Element(val tag: Int, val value: ByteArray) {
        fun intValue(): Int = value.fold(0) { acc, b -> (acc shl 8) or (b.toInt() and 0xFF) }
    }

    fun tlv(tag: Int, value: ByteArray): ByteArray = byteArrayOf(tag.toByte()) + length(value.size) + value

    fun length(n: Int): ByteArray = when {
        n < 0x80 -> byteArrayOf(n.toByte())
        n <= 0xFF -> byteArrayOf(0x81.toByte(), n.toByte())
        else -> byteArrayOf(0x82.toByte(), (n shr 8).toByte(), n.toByte())
    }

    /** Splits [buf] into consecutive single-byte-tag TLVs. */
    fun parse(buf: ByteArray): List<Element> {
        val out = mutableListOf<Element>()
        val r = Axdr.Reader(buf)
        while (r.remaining > 0) {
            val tag = r.u8()
            val first = r.u8()
            val len = if (first < 0x80) first else r.uint(first and 0x7F).toInt()
            out += Element(tag, r.bytes(len))
        }
        return out
    }
}
