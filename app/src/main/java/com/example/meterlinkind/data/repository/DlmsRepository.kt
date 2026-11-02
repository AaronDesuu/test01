package com.example.meterlinkind.data.repository

import android.annotation.SuppressLint
import android.util.Log
import com.example.meterlinkind.data.protocol.Association
import com.example.meterlinkind.data.protocol.AssociationResult
import com.example.meterlinkind.data.protocol.Axdr
import com.example.meterlinkind.data.protocol.ByteLink
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.data.protocol.Conformance
import com.example.meterlinkind.data.protocol.CosemAttribute
import com.example.meterlinkind.data.protocol.DataAccessResult
import com.example.meterlinkind.data.protocol.Get
import com.example.meterlinkind.data.protocol.GetResult
import com.example.meterlinkind.data.protocol.PublicObjects
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

    /** Client the current association was opened as; only Public (PC) for now. */
    private val client = ClientProfile.PUBLIC

    private val bleLink = object : ByteLink {
        override suspend fun send(data: ByteArray) = bleRepository.writeCharacteristic(data)
        override suspend fun receive(timeoutMs: Long) = bleRepository.waitForResponse(timeoutMs)
        override fun clear() = bleRepository.clearReceived()
    }

    private val transport = WrapperTransport(bleLink, client.clientPort)

    /** Result of the last successful AARQ/AARE, or null when not associated. */
    var association: AssociationResult? = null
        private set

    /**
     * Opens a DLMS association (AARQ/AARE); the wrapper has no link-layer step.
     * As the Public client this is a plain LN context with no authentication
     * or ciphering.
     */
    fun establishConnection(): Flow<ConnectionProgress> = flow {
        operationMutex.withLock {
            association = null
            emit(ConnectionProgress.Connecting("Associating as ${client.label}..."))
            val aare = try {
                transport.request(Association.publicRequest())
            } catch (e: TransportException) {
                emit(ConnectionProgress.Failed(e.message ?: "No association response"))
                return@withLock
            }

            val result = try {
                Association.parseResponse(aare)
            } catch (e: Exception) {
                Log.e(TAG, "Bad AARE ${aare.toHex()}", e)
                emit(ConnectionProgress.Failed("Bad association response: ${e.message}"))
                return@withLock
            }
            Log.d(TAG, "AARE: $result")

            if (!result.accepted) {
                emit(ConnectionProgress.Failed("Association rejected: ${result.reason}"))
                return@withLock
            }

            Log.i(
                TAG, "Associated as ${client.label}: conformance=" +
                    "${Conformance.describe(result.negotiatedConformance)}, " +
                    "server max PDU=${result.serverMaxReceivePdu}"
            )
            association = result
            emit(ConnectionProgress.Connected)
        }
    }

    /** Call when the BLE link drops: the association can't be used any more. */
    fun clearAssociation() {
        association = null
    }

    /**
     * Sends RLRQ so the meter frees the association at once instead of on its
     * inactivity timeout. Best effort: the caller drops the BLE link right after.
     */
    suspend fun release() {
        operationMutex.withLock {
            if (association == null) return@withLock
            association = null
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
     * display string. Follows Get-Response-With-Datablock if the meter splits
     * the value, although the Public client isn't granted block transfer.
     */
    fun readAttribute(target: CosemAttribute): Flow<MeterDataResult> = flow {
        operationMutex.withLock {
            emit(MeterDataResult.Loading)
            if (association == null) {
                emit(MeterDataResult.Error("Not associated"))
                return@withLock
            }
            Log.d(TAG, "GET ${target.name} (class ${target.classId}, ${target.obis}, attr ${target.attribute})")

            try {
                var response = Get.parseResponse(transport.request(Get.request(target)))
                val blocks = java.io.ByteArrayOutputStream()
                while (response is GetResult.Block) {
                    blocks.write(response.raw)
                    if (response.last) {
                        response = GetResult.Data(Axdr.decode(blocks.toByteArray()))
                        break
                    }
                    emit(MeterDataResult.Partial(arrayListOf("Receiving block ${response.blockNumber}...")))
                    response = Get.parseResponse(transport.request(Get.nextBlock(response.blockNumber)))
                }

                when (response) {
                    is GetResult.Data -> {
                        val text = response.data.display()
                        Log.d(TAG, "${target.name} = $text")
                        emit(MeterDataResult.Success(arrayListOf(text)))
                    }
                    is GetResult.AccessError -> emit(MeterDataResult.Error("Access error: ${response.reason}"))
                    is GetResult.BlockError ->
                        emit(MeterDataResult.Error("Block ${response.blockNumber} error: ${DataAccessResult.describe(response.code)}"))
                    is GetResult.ServiceFailure -> emit(MeterDataResult.Error(response.message))
                    is GetResult.Block -> Unit // unreachable: loop above consumes blocks
                }
            } catch (e: TransportException) {
                emit(MeterDataResult.Error(e.message ?: "No response"))
            } catch (e: Exception) {
                Log.e(TAG, "GET ${target.name} failed", e)
                emit(MeterDataResult.Error("Bad response: ${e.message}"))
            }
        }
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
