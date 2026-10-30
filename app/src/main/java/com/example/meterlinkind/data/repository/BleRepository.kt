package com.example.meterlinkind.data.repository

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothProfile
import android.bluetooth.BluetoothStatusCodes
import android.content.Context
import android.os.Build
import android.util.Log
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.withTimeoutOrNull
import java.util.UUID

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
@SuppressLint("MissingPermission")
class BleRepository(private val context: Context) {
    private val TAG = "BleRepository"
    private var bluetoothGatt: BluetoothGatt? = null
    private val SERVICE_UUID = "b973f2e0-b19e-11e2-9e96-0800200c9a66"
    private val READ_UUID = "d973f2e1-b19e-11e2-9e96-0800200c9a66"
    private val WRITE_UUID = "e973f2e2-b19e-11e2-9e96-0800200c9a66"
    private val CCCD_UUID = "00002902-0000-1000-8000-00805f9b34fb"

    private var _dataChannel = Channel<ByteArray>(Channel.UNLIMITED)
    private val _connectionState = MutableStateFlow<BleConnectionState>(BleConnectionState.Disconnected)
    val connectionState: StateFlow<BleConnectionState> = _connectionState.asStateFlow()

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "Connection state change failed with status: $status")
                _connectionState.value = BleConnectionState.Error("Connection failed (error $status)")
                gatt?.close()
                bluetoothGatt = null
                return
            }

            when (newState) {
                BluetoothProfile.STATE_CONNECTED -> {
                    Log.d(TAG, "Connected, discovering services...")
                    bluetoothGatt = gatt
                    gatt?.discoverServices()
                }
                BluetoothProfile.STATE_DISCONNECTED -> {
                    Log.d(TAG, "Disconnected")
                    _connectionState.value = BleConnectionState.Disconnected
                    gatt?.close()
                    bluetoothGatt = null
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            if (status != BluetoothGatt.GATT_SUCCESS) return
            Log.d(TAG, "Services discovered, enabling notifications")
            gatt ?: return
            val service = gatt.getService(UUID.fromString(SERVICE_UUID)) ?: return
            val characteristic = service.getCharacteristic(UUID.fromString(READ_UUID)) ?: return
            val descriptor = characteristic.getDescriptor(UUID.fromString(CCCD_UUID)) ?: return

            gatt.setCharacteristicNotification(characteristic, true)
            val value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                gatt.writeDescriptor(descriptor, value)
            } else {
                @Suppress("DEPRECATION")
                run {
                    descriptor.value = value
                    gatt.writeDescriptor(descriptor)
                }
            }
        }

        override fun onDescriptorWrite(gatt: BluetoothGatt?, descriptor: BluetoothGattDescriptor?, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d(TAG, "Notifications enabled, requesting MTU")
                gatt?.requestMtu(512)
            }
        }

        override fun onMtuChanged(gatt: BluetoothGatt?, mtu: Int, status: Int) {
            Log.d(TAG, "MTU changed to $mtu")
            _connectionState.value = BleConnectionState.Connected
        }

        // API 33+ delivers the value as a parameter; pre-33 still calls the legacy
        // override that reads characteristic.value. Both funnel into onDataReceived.
        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) {
            onDataReceived(value)
        }

        @Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")
        override fun onCharacteristicChanged(
            gatt: BluetoothGatt?,
            characteristic: BluetoothGattCharacteristic?
        ) {
            // Pre-API-33 callback; API 33+ goes through the three-arg override above.
            characteristic?.value?.let(::onDataReceived)
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt?,
            characteristic: BluetoothGattCharacteristic?,
            status: Int
        ) {
            Log.d(TAG, "onCharacteristicWrite: status=$status")
        }
    }

    private fun onDataReceived(data: ByteArray) {
        Log.d(TAG, "Data received: ${data.size} bytes - ${data.joinToString("") { "%02x".format(it) }}")
        _dataChannel.trySend(data)
    }

    @SuppressLint("MissingPermission")
    fun connectToDevice(device: BluetoothDevice) {
        _connectionState.value = BleConnectionState.Connecting
        // Clear old data channel and create a fresh one for new connection
        _dataChannel.close()
        _dataChannel = Channel(Channel.UNLIMITED)
        // Stash the gatt immediately so an early disconnect() can clean it up
        // even if STATE_CONNECTED hasn't arrived yet.
        bluetoothGatt = device.connectGatt(context, false, gattCallback)
    }

    @SuppressLint("MissingPermission")
    fun writeCharacteristic(data: ByteArray): Boolean {
        val gatt = bluetoothGatt ?: return false
        val service = gatt.getService(UUID.fromString(SERVICE_UUID)) ?: return false
        val characteristic = service.getCharacteristic(UUID.fromString(WRITE_UUID)) ?: return false

        Log.d(TAG, "Writing ${data.size} bytes")

        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            gatt.writeCharacteristic(
                characteristic,
                data,
                BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
            ) == BluetoothStatusCodes.SUCCESS
        } else {
            @Suppress("DEPRECATION")
            run {
                characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
                characteristic.value = data
                gatt.writeCharacteristic(characteristic)
            }
        }
    }

    suspend fun waitForResponse(timeoutMs: Long = 3000): ByteArray? {
        return withTimeoutOrNull(timeoutMs) {
            _dataChannel.receive()
        }
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        // Just request disconnect; onConnectionStateChange will close() and null out
        // bluetoothGatt when STATE_DISCONNECTED arrives.
        bluetoothGatt?.disconnect()
    }
}