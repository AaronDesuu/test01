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
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import android.util.Log
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
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

/**
 * GATT transport for the FT5AB's Dialog DSPS module (DA14531, `BLE_FT5AB`).
 *
 * The module is a transparent BLE<->UART bridge to the meter at 19200 baud.
 * Notifications are raw UART chunks, not frames: callers reassemble them
 * (see `WrapperAssembler`).
 */
@SuppressLint("MissingPermission")
class BleRepository(private val context: Context) {
    private val TAG = "BleRepository"
    private var bluetoothGatt: BluetoothGatt? = null

    companion object {
        // DSPS GATT table, BLE_FT5AB/.../spss/user_sps_config.h
        val SERVICE_UUID: UUID = UUID.fromString("bdcede40-50d8-4eea-9afd-9218bf8cc760")
        val TX_UUID: UUID = UUID.fromString("bdcede40-50d8-4eea-9afd-9218bf8cc761")         // meter -> phone, notify
        val RX_UUID: UUID = UUID.fromString("bdcede40-50d8-4eea-9afd-9218bf8cc762")         // phone -> meter, write w/o response
        val FLOW_CTRL_UUID: UUID = UUID.fromString("bdcede40-50d8-4eea-9afd-9218bf8cc763")  // 0x01 = resume, 0x02 = pause
        val CCCD_UUID: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")

        /** The FT5AB still advertises the F5LWF-era service UUID; used by the scanner. */
        val ADVERTISED_SERVICE_UUID: UUID = UUID.fromString("b973f2e0-b19e-11e2-9e96-0800200c9a66")

        private const val FLOW_ON: Byte = 0x01
        private const val FLOW_OFF: Byte = 0x02

        /** Android's ceiling; the DSPS firmware accepts up to 800. */
        private const val REQUESTED_MTU = 517
        /** SPS_SERVER_RX_CHAR_LEN: the RX characteristic holds at most 250 bytes. */
        private const val MAX_CHAR_LEN = 250
        private const val WRITE_CALLBACK_TIMEOUT_MS = 2000L
        private const val FLOW_RESUME_TIMEOUT_MS = 5000L
        /**
         * From connectGatt until the link is ready (services, notifications,
         * FLOW_ON, MTU). Android's own limit is about 30 s, too long in the field.
         */
        const val CONNECT_TIMEOUT_MS = 15_000L
        /**
         * Right after the MTU exchange the module renegotiates the connection
         * interval. A write that landed in that window (15 ms after connect) was
         * followed by the module going silent and a supervision timeout
         * (status 8), so the first write waits this long.
         */
        private const val LINK_SETTLE_MS = 500L
    }

    private var _dataChannel = Channel<ByteArray>(Channel.UNLIMITED)
    private val _connectionState = MutableStateFlow<BleConnectionState>(BleConnectionState.Disconnected)
    val connectionState: StateFlow<BleConnectionState> = _connectionState.asStateFlow()

    /** False while the meter has asked us to pause (flow-control notification 0x02). */
    private val canSend = MutableStateFlow(true)
    /** Signalled from onCharacteristicWrite so the next chunk is only queued once the stack is ready. */
    private val writeDone = Channel<Int>(Channel.CONFLATED)
    private var mtu = 23
    private var linkReadyAt = 0L

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            if (gatt != null && bluetoothGatt != null && gatt != bluetoothGatt) {
                // Late callback from a GATT replaced by connectToDevice(); don't touch the new one.
                gatt.close()
                return
            }
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
                    // Keep an Error set by fail() visible instead of masking it.
                    if (_connectionState.value !is BleConnectionState.Error) {
                        _connectionState.value = BleConnectionState.Disconnected
                    }
                    gatt?.close()
                    bluetoothGatt = null
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            gatt ?: return
            if (status != BluetoothGatt.GATT_SUCCESS) {
                fail(gatt, "Service discovery failed (error $status)")
                return
            }
            val service = gatt.getService(SERVICE_UUID)
            if (service == null) {
                fail(gatt, "Not an FT5AB meter: DSPS service not found")
                return
            }
            Log.d(TAG, "DSPS service found, enabling TX notifications")
            if (!enableNotifications(gatt, service.getCharacteristic(TX_UUID))) {
                fail(gatt, "Could not enable meter notifications")
            }
        }

        override fun onDescriptorWrite(gatt: BluetoothGatt?, descriptor: BluetoothGattDescriptor?, status: Int) {
            gatt ?: return
            val uuid = descriptor?.characteristic?.uuid
            if (status != BluetoothGatt.GATT_SUCCESS) {
                if (uuid == TX_UUID) {
                    fail(gatt, "Enabling notifications failed (error $status)")
                } else {
                    Log.w(TAG, "Flow-control notifications failed (error $status), continuing without")
                    gatt.requestMtu(REQUESTED_MTU)
                }
                return
            }
            // Descriptor writes must be serialized: TX first, then flow control, then MTU.
            when (uuid) {
                TX_UUID -> {
                    val flow = gatt.getService(SERVICE_UUID)?.getCharacteristic(FLOW_CTRL_UUID)
                    if (flow == null || !enableNotifications(gatt, flow)) {
                        Log.w(TAG, "Flow-control notifications unavailable, continuing without")
                        gatt.requestMtu(REQUESTED_MTU)
                    }
                }
                else -> {
                    // The DSPS server clears tx_flow_en on every disconnect and
                    // nothing sets it again, so after the first session it drops
                    // all meter->phone data until the client writes FLOW_ON
                    // (sps_server.c:sps_cleanup). Dialog's host does this on
                    // every connect (sps_client.c); so must we.
                    val flow = gatt.getService(SERVICE_UUID)?.getCharacteristic(FLOW_CTRL_UUID)
                    if (flow != null && writeChunk(gatt, flow, byteArrayOf(FLOW_ON))) {
                        Log.d(TAG, "Notifications enabled, sent FLOW_ON")
                        // onCharacteristicWrite for this write requests the MTU.
                    } else {
                        Log.w(TAG, "Could not send FLOW_ON, meter replies may be dropped")
                        gatt.requestMtu(REQUESTED_MTU)
                    }
                }
            }
        }

        override fun onMtuChanged(gatt: BluetoothGatt?, mtu: Int, status: Int) {
            Log.d(TAG, "MTU changed to $mtu (status $status)")
            if (status == BluetoothGatt.GATT_SUCCESS) this@BleRepository.mtu = mtu
            // Some phones settle on a slow connection interval (25 ms on a Galaxy A06), and the
            // meter's module then drops bytes from long replies. Ask for the fastest interval.
            val fast = gatt?.requestConnectionPriority(BluetoothGatt.CONNECTION_PRIORITY_HIGH)
            Log.d(TAG, "Requested high connection priority: $fast")
            linkReadyAt = SystemClock.elapsedRealtime() + LINK_SETTLE_MS
            _connectionState.value = BleConnectionState.Connected
        }

        // API 33+ delivers the value as a parameter; pre-33 still calls the legacy
        // override that reads characteristic.value. Both funnel into onNotification.
        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) {
            onNotification(characteristic.uuid, value)
        }

        @Suppress("DEPRECATION", "OVERRIDE_DEPRECATION")
        override fun onCharacteristicChanged(
            gatt: BluetoothGatt?,
            characteristic: BluetoothGattCharacteristic?
        ) {
            // Pre-API-33 callback; API 33+ goes through the three-arg override above.
            characteristic ?: return
            characteristic.value?.let { onNotification(characteristic.uuid, it) }
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt?,
            characteristic: BluetoothGattCharacteristic?,
            status: Int
        ) {
            if (characteristic?.uuid == FLOW_CTRL_UUID) {
                Log.d(TAG, "FLOW_ON written (status $status), requesting MTU")
                gatt?.requestMtu(REQUESTED_MTU)
            } else {
                writeDone.trySend(status)
            }
        }
    }

    private fun fail(gatt: BluetoothGatt, message: String) {
        Log.e(TAG, message)
        _connectionState.value = BleConnectionState.Error(message)
        gatt.disconnect()
    }

    private fun enableNotifications(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic?): Boolean {
        characteristic ?: return false
        val descriptor = characteristic.getDescriptor(CCCD_UUID) ?: return false
        if (!gatt.setCharacteristicNotification(characteristic, true)) return false
        val value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            gatt.writeDescriptor(descriptor, value) == BluetoothStatusCodes.SUCCESS
        } else {
            @Suppress("DEPRECATION")
            run {
                descriptor.value = value
                gatt.writeDescriptor(descriptor)
            }
        }
    }

    private fun onNotification(uuid: UUID, data: ByteArray) {
        when (uuid) {
            FLOW_CTRL_UUID -> {
                val on = data.firstOrNull() != FLOW_OFF
                Log.d(TAG, "Flow control: ${if (on) "resume" else "pause"}")
                canSend.value = on
            }
            else -> {
                Log.d(TAG, "RX ${data.size} bytes: ${data.joinToString("") { "%02x".format(it) }}")
                _dataChannel.trySend(data)
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun connectToDevice(device: BluetoothDevice) {
        mainHandler.removeCallbacks(connectTimeout) // a previous attempt's timer must not end this one
        _connectionState.value = BleConnectionState.Connecting
        // Clear old data channel and create a fresh one for new connection
        _dataChannel.close()
        _dataChannel = Channel(Channel.UNLIMITED)
        canSend.value = true
        mtu = 23
        // Never stack a second GATT client on a link that is still open.
        bluetoothGatt?.let {
            it.disconnect()
            it.close()
        }
        // Stash the gatt immediately so an early disconnect() can clean it up
        // even if STATE_CONNECTED hasn't arrived yet.
        bluetoothGatt = device.connectGatt(context, false, gattCallback, BluetoothDevice.TRANSPORT_LE)
        mainHandler.postDelayed(connectTimeout, CONNECT_TIMEOUT_MS)
    }

    private val mainHandler = Handler(Looper.getMainLooper())

    /**
     * Gives up on a connection still not ready after [CONNECT_TIMEOUT_MS]. The
     * GATT is closed outright: without a link, disconnect() may never call back.
     */
    @SuppressLint("MissingPermission")
    private val connectTimeout = Runnable {
        if (_connectionState.value !is BleConnectionState.Connecting) return@Runnable
        Log.w(TAG, "No connection after ${CONNECT_TIMEOUT_MS / 1000} s, giving up")
        bluetoothGatt?.let {
            it.disconnect()
            it.close()
        }
        bluetoothGatt = null
        _connectionState.value = BleConnectionState.Error(
            "Couldn't connect within ${CONNECT_TIMEOUT_MS / 1000} s. Move closer to the meter and try again."
        )
    }

    /**
     * Writes [data] to the meter, split into pieces that fit the MTU and the
     * 250-byte RX characteristic. Pieces go out back to back (each one waits only
     * for the stack's write callback) because the meter's wrapper receiver has an
     * inter-character timeout. Honours the meter's flow-control pause.
     */
    @SuppressLint("MissingPermission")
    suspend fun writeCharacteristic(data: ByteArray): Boolean {
        val gatt = bluetoothGatt ?: return false
        val characteristic = gatt.getService(SERVICE_UUID)?.getCharacteristic(RX_UUID) ?: return false
        val chunkSize = minOf(mtu - 3, MAX_CHAR_LEN)

        val settle = linkReadyAt - SystemClock.elapsedRealtime()
        if (settle > 0) delay(settle)

        Log.d(TAG, "TX ${data.size} bytes: ${data.joinToString("") { "%02x".format(it) }}")
        writeDone.tryReceive() // drop a stale callback

        var offset = 0
        while (offset < data.size) {
            if (!canSend.value) {
                Log.d(TAG, "Meter paused us, waiting for resume")
                withTimeoutOrNull(FLOW_RESUME_TIMEOUT_MS) { canSend.first { it } } ?: run {
                    Log.e(TAG, "Meter never resumed flow")
                    return false
                }
            }
            val end = minOf(offset + chunkSize, data.size)
            val chunk = data.copyOfRange(offset, end)
            if (!writeChunk(gatt, characteristic, chunk)) {
                Log.e(TAG, "Write of bytes $offset..$end failed")
                return false
            }
            val status = withTimeoutOrNull(WRITE_CALLBACK_TIMEOUT_MS) { writeDone.receive() }
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "Write callback status=$status for bytes $offset..$end")
                return false
            }
            offset = end
        }
        return true
    }

    @SuppressLint("MissingPermission")
    private fun writeChunk(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, chunk: ByteArray): Boolean =
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            gatt.writeCharacteristic(
                characteristic,
                chunk,
                BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
            ) == BluetoothStatusCodes.SUCCESS
        } else {
            @Suppress("DEPRECATION")
            run {
                characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
                characteristic.value = chunk
                gatt.writeCharacteristic(characteristic)
            }
        }

    /** Next raw notification chunk from the meter, or null on timeout. */
    suspend fun waitForResponse(timeoutMs: Long = 3000): ByteArray? {
        return withTimeoutOrNull(timeoutMs) {
            _dataChannel.receive()
        }
    }

    /** Discards any chunks still queued from an earlier exchange. */
    fun clearReceived() {
        while (_dataChannel.tryReceive().isSuccess) Unit
    }

    /** Drops the link because a layer above failed, keeping [message] as the visible state. */
    @SuppressLint("MissingPermission")
    fun failAndDisconnect(message: String) {
        _connectionState.value = BleConnectionState.Error(message)
        bluetoothGatt?.disconnect()
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        // Just request disconnect; onConnectionStateChange will close() and null out
        // bluetoothGatt when STATE_DISCONNECTED arrives.
        bluetoothGatt?.disconnect()
    }
}
