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

    suspend fun establishConnection(device: BluetoothDevice): Flow<ConnectionProgress> = flow {
        dlms.readMeterInformation()
        dlms.init(0x06, 0x06, 0x01, 0x01)

        emit(ConnectionProgress.Connecting("Opening HDLC..."))
        delay(500)

        // SNRM
        val openRequest = dlms.Open()
        Log.d(TAG, "Sending SNRM: ${openRequest.size} bytes")

        if (!bleRepository.writeCharacteristic(openRequest)) {
            emit(ConnectionProgress.Failed("Failed to send SNRM"))
            return@flow
        }

        val openResponse = bleRepository.waitForResponse(3000)
        if (openResponse == null) {
            emit(ConnectionProgress.Failed("No UA response"))
            return@flow
        }

        Log.d(TAG, "Received UA: ${openResponse.size} bytes")

        // AARQ
        val ret = IntArray(2)
        val sessionRequest = dlms.Session(ret, openResponse)

        if (ret[0] == 0 || sessionRequest == null) {
            emit(ConnectionProgress.Failed("Failed to create AARQ"))
            return@flow
        }

        Log.d(TAG, "Sending AARQ: ${sessionRequest.size} bytes")

        if (!bleRepository.writeCharacteristic(sessionRequest)) {
            emit(ConnectionProgress.Failed("Failed to send AARQ"))
            return@flow
        }

        val sessionResponse = bleRepository.waitForResponse(3000)
        if (sessionResponse == null) {
            emit(ConnectionProgress.Failed("No AARE response"))
            return@flow
        }

        Log.d(TAG, "Received AARE: ${sessionResponse.size} bytes")
        emit(ConnectionProgress.Connected)
    }
}

sealed class ConnectionProgress {
    data class Connecting(val message: String) : ConnectionProgress()
    object Connected : ConnectionProgress()
    data class Failed(val message: String) : ConnectionProgress()
}