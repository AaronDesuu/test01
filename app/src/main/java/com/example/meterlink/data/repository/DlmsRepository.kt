package com.example.meterlink.data.repository

import android.bluetooth.BluetoothDevice
import android.util.Log
import com.example.meterlink.dlms.DLMS
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import javax.inject.Inject

class DlmsRepository @Inject constructor(
    private val bleRepository: BleRepository,
    private val dlms: DLMS
) {
    private val TAG = "DlmsRepository"
    init {
        dlms.readMeterInformation()
        Log.d(TAG, "Loaded settings: Account=${dlms.Account(-1)}, Rank=${dlms.getRank(-1)}, Addr=${dlms.getAddress(-1)}")
    }
    suspend fun establishConnection(device: BluetoothDevice): Flow<ConnectionProgress> = flow {
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

        // Step 4: Challenge (for HLS authentication)
        val challengeRequest = dlms.Challenge(ret, sessionResponse)
        if (ret[0] == 0) {
            emit(ConnectionProgress.Failed("Failed to process AARE"))
            return@flow
        }

        if (challengeRequest != null) {
            // HLS required (SUPER/ADMIN)
            emit(ConnectionProgress.Connecting("HLS authentication..."))

            if (!bleRepository.writeCharacteristic(challengeRequest)) {
                emit(ConnectionProgress.Failed("Failed to send Challenge"))
                return@flow
            }

            // Step 6: Receive Challenge response and Confirm
            val challengeResponse = bleRepository.waitForResponse(3000)
            if (challengeResponse == null) {
                emit(ConnectionProgress.Failed("No Challenge response"))
                return@flow
            }

            val confirmRequest = dlms.Confirm(ret, challengeResponse)
            if (ret[0] == 0) {
                emit(ConnectionProgress.Failed("Failed HLS authentication"))
                return@flow
            }

            Log.d(TAG, "HLS authentication completed")
        } else {
            // No Challenge needed (READER/POWER/PUBLIC)
            Log.d(TAG, "LLS/No authentication")
        }

        emit(ConnectionProgress.Connected)
    }

    suspend fun readMeterData(objectIndex: Int, attribute: Byte = 2): Flow<MeterDataResult> = flow {
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
        val data = dlms.DataRes(ret, response, true)

        Log.d(TAG, "DataRes returned: size=${data.size}, ret[0]=${ret[0]}, ret[1]=${ret[1]}")
        data.forEachIndexed { index, s ->
            Log.d(TAG, "data[$index]: $s")
        }

        when {
            ret[1] == -2 -> emit(MeterDataResult.Error("Invalid HDLC frame"))
            ret[1] == -1 -> emit(MeterDataResult.Error("Service error"))
            ret[1] != 0 -> emit(MeterDataResult.Error("Access error: ${ret[1]}"))
            ret[0] == 2 -> emit(MeterDataResult.Partial(data))
            else -> emit(MeterDataResult.Success(data))
        }
    }

    fun updateSettings(account: String, password: String, address: String, rank: String, scan: String, tick: String, interval: String) {
        dlms.Account(account, -1)
        dlms.Password(password, -1)
        dlms.writeAddress(address, -1)
        dlms.writeRank(rank, -1)
        dlms.writeScan(scan)
        dlms.writeTick(tick)
        dlms.writeInterval(interval)
        dlms.updateAllMeterInformation() // Saves to file
    }

    fun getSettings(): Map<String, String> {
        // Load once when settings screen opens
        if (dlms.Count() == 0) {
            dlms.readMeterInformation()
        }

        return mapOf(
            "account" to (dlms.Account(-1) ?: ""),
            "password" to (dlms.Password(-1) ?: "3030303030303030"),
            "address" to (dlms.getAddress(-1) ?: "41"),
            "rank" to (dlms.getRank(-1) ?: "03"),
            "scan" to dlms.readScan(),
            "tick" to dlms.readTick(),
            "interval" to dlms.readInterval()
        )
    }

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