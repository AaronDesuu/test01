package com.example.meterlink.data.repository

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothProfile
import android.content.Context
import android.util.Log
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.suspendCancellableCoroutine
import java.util.UUID
import kotlin.coroutines.resume

sealed class BleConnectionState {
    object Disconnected : BleConnectionState()
    object Connecting : BleConnectionState()
    object Connected : BleConnectionState()
    data class Error(val message: String) : BleConnectionState()
}

sealed class BleDataResult {
    data class Success(val data: ByteArray) : BleDataResult()
    data class Error(val message: String) : BleDataResult()
}

class BleRepository(private val context: Context) {
    private val TAG = "BleRepository"
    private var bluetoothGatt: BluetoothGatt? = null

    /**
     * Connect to BLE device and return connection state flow
     */
    @SuppressLint("MissingPermission")
    fun connectToDevice(device: BluetoothDevice): Flow<BleConnectionState> = callbackFlow {
        val gattCallback = object : BluetoothGattCallback() {
            override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
                when (newState) {
                    BluetoothProfile.STATE_CONNECTED -> {
                        Log.d(TAG, "Connected to GATT server")
                        bluetoothGatt = gatt
                        gatt?.discoverServices()
                        trySend(BleConnectionState.Connected)
                    }
                    BluetoothProfile.STATE_DISCONNECTED -> {
                        Log.d(TAG, "Disconnected from GATT server")
                        trySend(BleConnectionState.Disconnected)
                        close()
                    }
                    BluetoothProfile.STATE_CONNECTING -> {
                        trySend(BleConnectionState.Connecting)
                    }
                }
            }

            override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    Log.d(TAG, "Services discovered")
                }
            }
        }

        try {
            trySend(BleConnectionState.Connecting)
            device.connectGatt(context, false, gattCallback)
        } catch (e: Exception) {
            trySend(BleConnectionState.Error(e.message ?: "Connection failed"))
            close()
        }

        awaitClose {
            bluetoothGatt?.disconnect()
            bluetoothGatt?.close()
            bluetoothGatt = null
        }
    }

    /**
     * Write data to characteristic
     */
    @SuppressLint("MissingPermission")
    suspend fun writeCharacteristic(
        serviceUuid: String,
        characteristicUuid: String,
        data: ByteArray
    ): BleDataResult = suspendCancellableCoroutine { continuation ->
        val gatt = bluetoothGatt
        if (gatt == null) {
            continuation.resume(BleDataResult.Error("Not connected"))
            return@suspendCancellableCoroutine
        }

        val service = gatt.getService(UUID.fromString(serviceUuid))
        val characteristic = service?.getCharacteristic(UUID.fromString(characteristicUuid))

        if (characteristic == null) {
            continuation.resume(BleDataResult.Error("Characteristic not found"))
            return@suspendCancellableCoroutine
        }

        val callback = object : BluetoothGattCallback() {
            override fun onCharacteristicWrite(
                gatt: BluetoothGatt?,
                characteristic: BluetoothGattCharacteristic?,
                status: Int
            ) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    continuation.resume(BleDataResult.Success(data))
                } else {
                    continuation.resume(BleDataResult.Error("Write failed with status $status"))
                }
            }
        }

        characteristic.value = data
        if (!gatt.writeCharacteristic(characteristic)) {
            continuation.resume(BleDataResult.Error("Write initiation failed"))
        }
    }

    /**
     * Read data from characteristic
     */
    @SuppressLint("MissingPermission")
    suspend fun readCharacteristic(
        serviceUuid: String,
        characteristicUuid: String
    ): BleDataResult = suspendCancellableCoroutine { continuation ->
        val gatt = bluetoothGatt
        if (gatt == null) {
            continuation.resume(BleDataResult.Error("Not connected"))
            return@suspendCancellableCoroutine
        }

        val service = gatt.getService(UUID.fromString(serviceUuid))
        val characteristic = service?.getCharacteristic(UUID.fromString(characteristicUuid))

        if (characteristic == null) {
            continuation.resume(BleDataResult.Error("Characteristic not found"))
            return@suspendCancellableCoroutine
        }

        val callback = object : BluetoothGattCallback() {
            override fun onCharacteristicRead(
                gatt: BluetoothGatt?,
                characteristic: BluetoothGattCharacteristic?,
                status: Int
            ) {
                if (status == BluetoothGatt.GATT_SUCCESS && characteristic != null) {
                    continuation.resume(BleDataResult.Success(characteristic.value))
                } else {
                    continuation.resume(BleDataResult.Error("Read failed with status $status"))
                }
            }
        }

        if (!gatt.readCharacteristic(characteristic)) {
            continuation.resume(BleDataResult.Error("Read initiation failed"))
        }
    }

    /**
     * Disconnect from device
     */
    @SuppressLint("MissingPermission")
    fun disconnect() {
        bluetoothGatt?.disconnect()
        bluetoothGatt?.close()
        bluetoothGatt = null
    }
}