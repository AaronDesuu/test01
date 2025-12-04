package com.example.meterlink.data.repository

import android.util.Log
import com.example.meterlink.dlms.DLMS
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import javax.inject.Inject

class DlmsRepository @Inject constructor(
    private val bleRepository: BleRepository,
    private val dlms: DLMS,
    private val settingsRepository: SettingsRepository
) {
    private val TAG = "DlmsRepository"
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
    fun establishConnection(): Flow<ConnectionProgress> = flow {
        Log.d(TAG, "Starting connection with Count=${dlms.Count()}")
        // Only call changeCurrent if we have data
        if (dlms.Count() > 0) {
            dlms.changeCurrent(0)
        }

        dlms.init(0x06, 0x06, 0x01, 0x01)

        emit(ConnectionProgress.Connecting("Opening HDLC..."))
        delay(500)

        // Step 0: SNRM
        val openRequest = dlms.Open()
        if (!bleRepository.writeCharacteristic(openRequest)) {
            emit(ConnectionProgress.Failed("Failed to send SNRM"))
            return@flow
        }

        // Step 2: Receive UA
        val openResponse = bleRepository.waitForResponse(3000)
        if (openResponse == null) {
            emit(ConnectionProgress.Failed("No UA response"))
            return@flow
        }

        Log.d(TAG, "Using rank: ${dlms.getRank(-1)}")

        // Step 2: AARQ
        val ret = IntArray(2)
        val sessionRequest = dlms.Session(ret, openResponse)

        if (ret[0] == 0 || sessionRequest == null) {
            emit(ConnectionProgress.Failed("Failed to create AARQ"))
            return@flow
        }

        if (!bleRepository.writeCharacteristic(sessionRequest)) {
            emit(ConnectionProgress.Failed("Failed to send AARQ"))
            return@flow
        }

        // Step 4: Receive AARE and check if Challenge needed
        val sessionResponse = bleRepository.waitForResponse(3000)
        if (sessionResponse == null) {
            emit(ConnectionProgress.Failed("No AARE response"))
            return@flow
        }

        Log.d(TAG, "Processing AARE response: ${sessionResponse.size} bytes")
        val challengeRequest = dlms.Challenge(ret, sessionResponse)
        Log.d(TAG, "Challenge result: ret[0]=${ret[0]}, request=${challengeRequest?.size ?: "null"}")

        if (ret[0] == 0) {
            emit(ConnectionProgress.Failed("Failed to process AARE"))
            return@flow
        }

        if (challengeRequest != null) {
            emit(ConnectionProgress.Connecting("HLS authentication..."))

            if (!bleRepository.writeCharacteristic(challengeRequest)) {
                emit(ConnectionProgress.Failed("Failed to send Challenge"))
                return@flow
            }

            val challengeResponse = bleRepository.waitForResponse(5000)
            if (challengeResponse == null) {
                emit(ConnectionProgress.Failed("No Challenge response"))
                return@flow
            }

            Log.d(TAG, "Processing Challenge response: ${challengeResponse.size} bytes")
            dlms.Confirm(ret, challengeResponse)
            Log.d(TAG, "Confirm result: ret[0]=${ret[0]}")

            if (ret[0] == 0) {
                emit(ConnectionProgress.Failed("Failed HLS authentication"))
                return@flow
            }

            Log.d(TAG, "HLS authentication completed")
        }

        emit(ConnectionProgress.Connected)
    }

    fun readSerialNumber(): Flow<MeterDataResult> = flow {
        Log.d(TAG, "Reading serial number (IST_APPROVAL_NO)")
        emit(MeterDataResult.Loading)

        val request = dlms.getReq(DLMS.IST_APPROVAL_NO, 2, 0, null, 0)  // Changed to IST_APPROVAL_NO
        Log.d(TAG, "Sending serial number request")

        if (!bleRepository.writeCharacteristic(request)) {
            Log.e(TAG, "Failed to write serial number request")
            emit(MeterDataResult.Error("Failed to send request"))
            return@flow
        }

        val response = bleRepository.waitForResponse(3000)
        if (response == null) {
            Log.e(TAG, "No serial number response")
            emit(MeterDataResult.Error("No response"))
            return@flow
        }

        val ret = IntArray(2)
        val data = dlms.DataRes(ret, response, true)
        Log.d(TAG, "Serial number data: ${data.joinToString()}, ret[1]=${ret[1]}")

        when {
            ret[1] != 0 -> emit(MeterDataResult.Error("Access error: ${ret[1]}"))
            else -> emit(MeterDataResult.Success(data))
        }
    }

    fun readMeterData(objectIndex: Int, attribute: Byte = 2): Flow<MeterDataResult> = flow {
        emit(MeterDataResult.Loading)

        val request = dlms.getReq(objectIndex, attribute, 0, null, 0)
        Log.d(TAG, "Reading object $objectIndex, attr $attribute")

        if (!bleRepository.writeCharacteristic(request)) {
            emit(MeterDataResult.Error("Failed to send GET request"))
            return@flow
        }

        val response = bleRepository.waitForResponse(3000)
        if (response == null) {
            emit(MeterDataResult.Error("No response"))
            return@flow
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
                return@flow
            }

            val nextResponse = bleRepository.waitForResponse(3000)
            if (nextResponse == null) {
                emit(MeterDataResult.Error("No response for next block"))
                return@flow
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
        emit(MeterDataResult.Loading)

        val request = dlms.actReq(DLMS.IST_DEMAND_RESET, 1, "0f00", 0)
        Log.d(TAG, "Sending demand reset")

        if (!bleRepository.writeCharacteristic(request)) {
            emit(MeterDataResult.Error("Failed to send ACTION"))
            return@flow
        }

        val response = bleRepository.waitForResponse(3000)
        if (response == null) {
            emit(MeterDataResult.Error("No response"))
            return@flow
        }

        val ret = IntArray(2)

        Log.d(TAG, "ACTION result: ret[0]=${ret[0]}, ret[1]=${ret[1]}")

        when {
            ret[1] == 12 -> emit(MeterDataResult.Error("Type unmatched (12) - Check parameter format"))
            ret[1] != 0 -> emit(MeterDataResult.Error("Error code: ${ret[1]}"))
            else -> emit(MeterDataResult.Success(arrayListOf("Demand reset success")))
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