package com.example.meterlinkind.data.repository

import android.annotation.SuppressLint
import android.util.Log
import com.example.meterlinkind.data.model.ClientSecurity
import com.example.meterlinkind.data.protocol.Action
import com.example.meterlinkind.data.protocol.ActionResult
import com.example.meterlinkind.data.protocol.Association
import com.example.meterlinkind.data.protocol.AssociationResult
import com.example.meterlinkind.data.protocol.Axdr
import com.example.meterlinkind.data.protocol.ByteLink
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.data.protocol.Conformance
import com.example.meterlinkind.data.protocol.CosemAttribute
import com.example.meterlinkind.data.protocol.CosemMethod
import com.example.meterlinkind.data.protocol.CosemSet
import com.example.meterlinkind.data.protocol.CosemStructures
import com.example.meterlinkind.data.protocol.DataAccessResult
import com.example.meterlinkind.data.protocol.DlmsData
import com.example.meterlinkind.data.protocol.GbtAssembler
import com.example.meterlinkind.data.protocol.GbtBlock
import com.example.meterlinkind.data.protocol.Get
import com.example.meterlinkind.data.protocol.GetResult
import com.example.meterlinkind.data.protocol.InterfaceClass
import com.example.meterlinkind.data.protocol.Mechanism
import com.example.meterlinkind.data.protocol.ObisCode
import com.example.meterlinkind.data.protocol.ObjectListEntry
import com.example.meterlinkind.data.protocol.PublicObjects
import com.example.meterlinkind.data.protocol.SecureChannel
import com.example.meterlinkind.data.protocol.ServiceError
import com.example.meterlinkind.data.protocol.TransportException
import com.example.meterlinkind.data.protocol.WrapperTransport
import com.example.meterlinkind.data.protocol.toHex
import com.example.meterlinkind.dlms.DLMS
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import javax.inject.Inject

class DlmsRepository @Inject constructor(
    private val bleRepository: BleRepository,
    private val dlms: DLMS,
    private val settingsRepository: SettingsRepository
) {
    private val TAG = "DlmsRepository"

    /** The meter answers within ~0.2 s; a silent one shouldn't hold up a disconnect. */
    private val RELEASE_TIMEOUT_MS = 1000L

    // Serializes every BLE write+wait sequence across the whole app. DLMS is
    // request/response and the data channel is shared, so two operations in
    // flight at once will scramble framing.
    private val operationMutex = Mutex()

    init {
        // Load from SharedPreferences and apply to DLMS
        val settings = settingsRepository.getSettings()
        dlms.readMeterInformation()
        dlms.Account(settings["account"]!!, -1)
        dlms.Password(settings["password"]!!, -1)
        dlms.writeAddress(settings["address"]!!, -1)
        dlms.writeRank(settings["rank"]!!, -1)
        dlms.updateAllMeterInformation()
    }

    /**
     * Our calling-AP-title, the IV prefix of every APDU we cipher. Only the
     * Super client's is checked (`acse.c:request_application_title`); this is
     * the title `dlms.c USERS[]` lists for MR and US.
     */
    private val CLIENT_SYSTEM_TITLE = "CLIENT00".toByteArray(Charsets.US_ASCII)

    private val CURRENT_ASSOCIATION_OBJECT_LIST = CosemAttribute(
        InterfaceClass.ASSOCIATION_LN, ObisCode.parse("0.0.40.0.0.255"), 2, "Current association.object_list"
    )

    private val bleLink = object : ByteLink {
        override suspend fun send(data: ByteArray) = bleRepository.writeCharacteristic(data)
        override suspend fun receive(timeoutMs: Long) = bleRepository.waitForResponse(timeoutMs)
        override fun clear() = bleRepository.clearReceived()
    }

    /** Client the current (or last) association was opened as. */
    var client: ClientProfile = ClientProfile.PUBLIC
        private set

    private var transport = WrapperTransport(bleLink, client.clientPort)

    /** Ciphers xDLMS APDUs of an MR or US association; null for Public. */
    private var channel: SecureChannel? = null

    /** Keys of the current association, to log in again after the meter's inactivity timeout. */
    private var security: ClientSecurity? = null

    /** Result of the last successful AARQ/AARE, or null when not associated. */
    var association: AssociationResult? = null
        private set

    /**
     * Opens a DLMS association as [client]; the wrapper has no link-layer step.
     * Public: plain LN context. MR: LLS password, encrypted APDUs. US: HLS
     * challenge exchange after the AARE, encrypted and authenticated APDUs.
     */
    fun establishConnection(client: ClientProfile, security: ClientSecurity): Flow<ConnectionProgress> = flow {
        operationMutex.withLock {
            association = null
            channel = null
            this@DlmsRepository.client = client
            this@DlmsRepository.security = security
            transport = WrapperTransport(bleLink, client.clientPort)
            emit(ConnectionProgress.Connecting("Associating as ${client.label}..."))

            val failure = try {
                associate(client, security) { emit(ConnectionProgress.Connecting(it)) }
            } catch (e: TransportException) {
                e.message ?: "No association response"
            } catch (e: Exception) {
                Log.e(TAG, "Association as ${client.label} failed", e)
                "Bad association response: ${e.message}"
            }
            if (failure != null) {
                association = null
                channel = null
                emit(ConnectionProgress.Failed(failure))
                return@withLock
            }

            val result = association!!
            Log.i(
                TAG, "Associated as ${client.label}: conformance=" +
                    "${Conformance.describe(result.negotiatedConformance)}, " +
                    "server max PDU=${result.serverMaxReceivePdu}"
            )
            emit(ConnectionProgress.Connected)
        }
    }

    /** Runs the AARQ/AARE (and HLS passes 3-4); sets [association] and returns null on success. */
    private suspend fun associate(
        client: ClientProfile,
        security: ClientSecurity,
        progress: suspend (String) -> Unit,
    ): String? {
        if (client.mechanism == Mechanism.NONE) {
            val result = Association.parseResponse(transport.request(Association.publicRequest()))
            Log.d(TAG, "AARE: $result")
            if (!result.accepted) return "Association rejected: ${result.reason}"
            association = result
            return null
        }

        if (!security.isValid) return "Keys must be ${ClientSecurity.KEY_BYTES} bytes each"
        val secure = SecureChannel(
            CLIENT_SYSTEM_TITLE, security.cipherKeys, client.securityControl, settingsRepository::nextFrameCounter
        )
        val clientChallenge = if (client.mechanism == Mechanism.HLS) randomChallenge() else null
        val aarq = Association.cipheredRequest(
            CLIENT_SYSTEM_TITLE,
            client.mechanism,
            clientChallenge ?: security.passwordBytes,
            secure.protect(Association.initiateRequest()),
        )
        val result = Association.parseResponse(transport.request(aarq), secure)
        Log.d(TAG, "AARE: $result")
        if (!result.accepted) return "Association rejected: ${result.reason}"
        channel = secure

        if (clientChallenge != null) {
            val serverChallenge = result.serverChallenge ?: return "Meter sent no HLS challenge"
            progress("Authenticating (HLS)...")
            val reply = Action.parseResponse(
                exchange(Association.hlsReplyRequest(security.passwordBytes, serverChallenge))
            )
            if (!Association.hlsVerify(security.passwordBytes, clientChallenge, reply)) {
                return when (reply) {
                    is ActionResult.Failure -> "HLS rejected (${reply.reason}): check the authentication password"
                    is ActionResult.ServiceFailure -> "HLS rejected: ${reply.message}"
                    is ActionResult.Success -> "Meter's HLS reply doesn't match: check the authentication password"
                }
            }
        }
        association = result
        return null
    }

    /** 16 printable characters, like the meter's own challenge (`aes_gcm.c:set_BtoA`). */
    private fun randomChallenge(): ByteArray {
        val random = java.security.SecureRandom()
        return ByteArray(16) { (0x21 + random.nextInt(0x7E - 0x21 + 1)).toByte() }
    }

    /**
     * Sends one xDLMS APDU, ciphered when the association is, and returns the
     * plain reply. A reply split into General Block Transfer blocks is
     * collected first (streamed blocks arrive unasked; otherwise we ack).
     */
    private suspend fun exchange(
        apdu: ByteArray,
        timeoutMs: Long = WrapperTransport.RESPONSE_TIMEOUT_MS,
    ): ByteArray {
        val secure = channel
        var reply = transport.request(secure?.protect(apdu) ?: apdu, timeoutMs)
        if ((reply.firstOrNull()?.toInt()?.and(0xFF)) == GbtBlock.TAG) reply = collectGbt(GbtBlock.parse(reply))
        return secure?.unprotect(reply) ?: reply
    }

    /**
     * [exchange] for the first request of an operation. The meter drops the
     * association after its inactivity timeout and then answers everything
     * with a confirmed-service-error (`0E 01 03 02`, `hrdtushin.c:local_call1`).
     * On that answer, log in again with the same client and keys and retry once.
     */
    private suspend fun exchangeInSession(apdu: ByteArray): ByteArray {
        val reply = exchange(apdu)
        if ((reply.firstOrNull()?.toInt()?.and(0xFF)) != ServiceError.TAG_CONFIRMED_SERVICE_ERROR) return reply
        val keys = security ?: return reply
        Log.i(TAG, "Meter ended the session (${reply.toHex()}); associating again as ${client.label}")
        channel = null
        val failure = try {
            associate(client, keys) {}
        } catch (e: Exception) {
            e.message ?: "no response"
        }
        if (failure != null) {
            association = null
            throw TransportException("The meter ended the session and logging in again failed ($failure). Reconnect.")
        }
        return exchange(apdu)
    }

    private suspend fun collectGbt(first: GbtBlock): ByteArray {
        val blocks = GbtAssembler()
        var block = first
        var ourBlockNumber = 0
        while (true) {
            blocks.add(block)?.let { return it }
            val next = if (block.streaming) {
                transport.receive()
            } else {
                transport.request(GbtBlock.ack(++ourBlockNumber, block.blockNumber))
            }
            block = GbtBlock.parse(next)
        }
    }

    /** Call when the BLE link drops: the association can't be used any more. */
    fun clearAssociation() {
        association = null
        channel = null
    }

    /**
     * Sends RLRQ so the meter frees the association at once instead of on its
     * inactivity timeout. Best effort: the caller drops the BLE link right after.
     */
    suspend fun release() {
        operationMutex.withLock {
            if (association == null) return@withLock
            association = null
            channel = null
            try {
                val reply = transport.request(Association.releaseRequest(), RELEASE_TIMEOUT_MS)
                Log.d(TAG, if (Association.parseReleaseResponse(reply)) "Association released" else "Meter refused release")
            } catch (e: Exception) {
                Log.w(TAG, "Release failed: ${e.message}")
            }
        }
    }

    fun readSerialNumber(): Flow<MeterDataResult> = readAttribute(PublicObjects.SERIAL_NUMBER)

    /**
     * GETs one attribute. Emits [MeterDataResult.Success] with a single
     * display string; [read] is the same with the decoded value.
     */
    fun readAttribute(target: CosemAttribute): Flow<MeterDataResult> = flow {
        emit(MeterDataResult.Loading)
        when (val result = read(target) { emit(MeterDataResult.Partial(arrayListOf(it))) }) {
            is AttributeRead.Value -> emit(MeterDataResult.Success(arrayListOf(result.data.display())))
            is AttributeRead.Failure -> emit(MeterDataResult.Error(result.message))
        }
    }

    /**
     * GETs one attribute, with [access] as selective access (e.g.
     * [SelectiveAccess.byEntry]) if given. Follows Get-Response-With-Datablock,
     * reporting each block through [progress].
     */
    suspend fun read(
        target: CosemAttribute,
        access: ByteArray? = null,
        progress: suspend (String) -> Unit = {},
    ): AttributeRead = operationMutex.withLock {
        if (association == null) return@withLock AttributeRead.Failure("Not connected to a meter")
        Log.d(TAG, "GET ${target.name} (class ${target.classId}, ${target.obis}, attr ${target.attribute})")
        try {
            var response = Get.parseResponse(exchangeInSession(Get.request(target, access)))
            val blocks = java.io.ByteArrayOutputStream()
            while (response is GetResult.Block) {
                blocks.write(response.raw)
                if (response.last) {
                    response = GetResult.Data(Axdr.decode(blocks.toByteArray()))
                    break
                }
                progress("Receiving block ${response.blockNumber}...")
                response = Get.parseResponse(exchange(Get.nextBlock(response.blockNumber)))
            }
            when (response) {
                is GetResult.Data -> AttributeRead.Value(response.data).also {
                    Log.d(TAG, "${target.name} = ${response.data.display().take(200)}")
                }
                is GetResult.AccessError -> AttributeRead.Failure(response.reason)
                is GetResult.BlockError ->
                    AttributeRead.Failure("Block ${response.blockNumber}: ${DataAccessResult.describe(response.code)}")
                is GetResult.ServiceFailure -> AttributeRead.Failure(response.message)
                is GetResult.Block -> AttributeRead.Failure("Unfinished block transfer") // loop above consumes blocks
            }
        } catch (e: TransportException) {
            AttributeRead.Failure(e.message ?: "No response")
        } catch (e: Exception) {
            Log.e(TAG, "GET ${target.name} failed", e)
            AttributeRead.Failure("Bad response: ${e.message}")
        }
    }

    /** SETs one attribute to [value] (one encoded `Data`); null on success, else the reason. */
    /** ACTIONs [target] with [parameter] (one encoded `Data`, or null); null on success, else the reason. */
    suspend fun invoke(target: CosemMethod, parameter: ByteArray?): String? = operationMutex.withLock {
        if (association == null) return@withLock "Not connected to a meter"
        Log.d(TAG, "ACTION ${target.name} (class ${target.classId}, ${target.obis}, method ${target.method})")
        try {
            when (val result = Action.parseResponse(exchangeInSession(Action.request(target, parameter)))) {
                is ActionResult.Success -> null
                is ActionResult.Failure -> result.reason
                is ActionResult.ServiceFailure -> result.message
            }
        } catch (e: TransportException) {
            e.message ?: "No response"
        } catch (e: Exception) {
            Log.e(TAG, "ACTION ${target.name} failed", e)
            "Bad response: ${e.message}"
        }
    }

    suspend fun write(target: CosemAttribute, value: ByteArray): String? = operationMutex.withLock {
        if (association == null) return@withLock "Not connected to a meter"
        Log.d(TAG, "SET ${target.name} (class ${target.classId}, ${target.obis}, attr ${target.attribute})")
        try {
            CosemSet.parseResponse(exchangeInSession(CosemSet.request(target, value)))
        } catch (e: TransportException) {
            e.message ?: "No response"
        } catch (e: Exception) {
            Log.e(TAG, "SET ${target.name} failed", e)
            "Bad response: ${e.message}"
        }
    }

    /** The objects the current association may see (current association, attribute 2). */
    suspend fun readObjectList(progress: suspend (String) -> Unit = {}): Result<List<ObjectListEntry>> =
        when (val result = read(CURRENT_ASSOCIATION_OBJECT_LIST, progress = progress)) {
            is AttributeRead.Value -> runCatching { CosemStructures.objectList(result.data) }
            is AttributeRead.Failure -> Result.failure(Exception(result.message))
        }

    fun readMeterData(objectIndex: Int, attribute: Byte = 2): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)

            val request = dlms.getReq(objectIndex, attribute, 0, null, 0)
            Log.d(TAG, "Reading object $objectIndex, attr $attribute")

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send GET request"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            val allData = ArrayList<String>()
            var data = dlms.DataRes(ret, response, true)
            allData.addAll(data)

            // Handle block transfer
            while (ret[0] == 2 && ret[1] == 0) {
                emit(MeterDataResult.Partial(ArrayList(allData)))

                val blockNo = dlms.blockNo
                val nextRequest = dlms.getReq_next(blockNo)

                if (!bleRepository.writeCharacteristic(nextRequest)) {
                    emit(MeterDataResult.Error("Failed to send GET-NEXT"))
                    return@withLock
                }

                val nextResponse = bleRepository.waitForResponse(3000)
                if (nextResponse == null) {
                    emit(MeterDataResult.Error("No response for next block"))
                    return@withLock
                }

                data = dlms.DataRes(ret, nextResponse, true)
                allData.addAll(data)
            }

            when {
                ret[1] < 0 -> emit(MeterDataResult.Error("Protocol error: ${ret[1]}"))
                ret[1] != 0 -> emit(MeterDataResult.Error("Access error: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(allData))
            }
        }
    }

    fun updateSettings(account: String, password: String, address: String, rank: String, scan: String, tick: String, interval: String) {
        settingsRepository.saveSettings(account, password, address, rank, scan, tick, interval)
        // Also update DLMS
        dlms.Account(account, -1)
        dlms.Password(password, -1)
        dlms.writeAddress(address, -1)
        dlms.writeRank(rank, -1)
        dlms.updateAllMeterInformation()
    }

    fun executeDemandReset(): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)

            val request = dlms.actReq(DLMS.IST_DEMAND_RESET, 1, "0f00", 0)
            Log.d(TAG, "Sending demand reset")

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send ACTION"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            Log.d(TAG, "ACTION result: ret[0]=${ret[0]}, ret[1]=${ret[1]}")

            when {
                ret[1] == 12 -> emit(MeterDataResult.Error("Type unmatched (12) - Check parameter format"))
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("Demand reset success")))
            }
        }
    }

    fun executeAction(objectIndex: Int, methodId: Int = 1, parameter: String = "0f00"): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            Log.d(TAG, "Executing action on object $objectIndex, method $methodId, param $parameter")

            val request = dlms.actReq(objectIndex, methodId.toByte(), parameter, 0)

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send ACTION"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            Log.d(TAG, "ACTION result: ret[0]=${ret[0]}, ret[1]=${ret[1]}")

            when {
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("Action completed successfully")))
            }
        }
    }

    @SuppressLint("DefaultLocale")
    fun setDateTime(): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            Log.d(TAG, "Setting clock to current time")

            // Get current time
            val calendar = java.util.Calendar.getInstance()
            val dateTime = String.format(
                "%04d%02d%02d%02d%02d%02d",
                calendar.get(java.util.Calendar.YEAR),
                calendar.get(java.util.Calendar.MONTH) + 1,
                calendar.get(java.util.Calendar.DAY_OF_MONTH),
                calendar.get(java.util.Calendar.HOUR_OF_DAY),
                calendar.get(java.util.Calendar.MINUTE),
                calendar.get(java.util.Calendar.SECOND)
            )

            val request = dlms.setReq(DLMS.IST_DATETIME_NOW, 2.toByte(), 0.toByte(), dateTime, 0)

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send SET"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            Log.d(TAG, "Set clock result: ret[1]=${ret[1]}")

            when {
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("Clock set successfully")))
            }
        }
    }

    fun setMeterType(): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            Log.d(TAG, "Setting meter type")

            // This needs to cycle through meter type settings (Imp-Exp/ABS/NET)
            // You may need to read current type first, then set next type
            val request = dlms.setReq(DLMS.IST_TYPE, 2.toByte(), 0.toByte(), "", 0)

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send SET"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            when {
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("Meter type configured")))
            }
        }
    }

    fun wholeClearMeasuredData(): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            Log.d(TAG, "Clearing all measured data")

            // Reset absolute energy data
            val request = dlms.actReq(DLMS.IST_ABS_ENERGY, 1.toByte(), "0f00", 0)

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send ACTION"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            when {
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("All measured data cleared")))
            }
        }
    }

    fun setMissingNeutral(enable: Boolean): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            Log.d(TAG, "Setting missing neutral detection: $enable")

            // Set missing neutral detection ON/OFF
            val parameter = if (enable) "01" else "00"
            val request = dlms.setReq(DLMS.IST_DETECT, 2.toByte(), 0.toByte(), parameter, 0)

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send SET"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            when {
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("Missing neutral ${if (enable) "enabled" else "disabled"}")))
            }
        }
    }

    @SuppressLint("DefaultLocale")
    fun setPowerNetwork(networkType: Int): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            Log.d(TAG, "Setting power network type: $networkType")

            // Set power network configuration (1 or 2)
            val parameter = String.format("%02d", networkType)
            val request = dlms.setReq(DLMS.IST_MODEL, 2.toByte(), 0.toByte(), parameter, 0)

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send SET"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            when {
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("Power network set to $networkType")))
            }
        }
    }

    fun setPotential(): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            Log.d(TAG, "Setting potential configuration")

            // Configure potential settings
            val request = dlms.actReq(DLMS.IST_CAL_VOLTAMP, 1.toByte(), "0f00", 0)

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send ACTION"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            when {
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("Potential configured")))
            }
        }
    }

    fun setTypeSet(): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            Log.d(TAG, "Setting type configuration")

            // Configure type settings
            val request = dlms.setReq(DLMS.IST_TYPE, 2.toByte(), 0.toByte(), "", 0)

            if (!bleRepository.writeCharacteristic(request)) {
                emit(MeterDataResult.Error("Failed to send SET"))
                return@withLock
            }

            val response = bleRepository.waitForResponse(3000)
            if (response == null) {
                emit(MeterDataResult.Error("No response"))
                return@withLock
            }

            val ret = IntArray(2)
            dlms.DataRes(ret, response, true)

            when {
                ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
                else -> emit(MeterDataResult.Success(arrayListOf("Type set configured")))
            }
        }
    }

    fun getSettings() = settingsRepository.getSettings()

    sealed class MeterDataResult {
        object Loading : MeterDataResult()
        data class Success(val data: ArrayList<String>) : MeterDataResult()
        data class Partial(val data: ArrayList<String>) : MeterDataResult()
        data class Error(val message: String) : MeterDataResult()
    }
}

sealed class ConnectionProgress {
    data class Connecting(val message: String) : ConnectionProgress()
    object Connected : ConnectionProgress()
    data class Failed(val message: String) : ConnectionProgress()
}

/** Outcome of one GET: the decoded value, or why there is none. */
sealed class AttributeRead {
    data class Value(val data: DlmsData) : AttributeRead()
    data class Failure(val message: String) : AttributeRead()
}
