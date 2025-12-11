package com.example.meterlink.presentation.viewmodel

import android.bluetooth.BluetoothDevice
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.meterlink.data.repository.BleConnectionState
import com.example.meterlink.data.repository.BleRepository
import com.example.meterlink.data.repository.ConnectionProgress
import com.example.meterlink.data.repository.DlmsRepository
import com.example.meterlink.dlms.DLMS
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import javax.inject.Inject

data class MeterConnectionUiState(
    val connectionState: BleConnectionState = BleConnectionState.Disconnected,
    val selectedDevice: BluetoothDevice? = null,
    val isDlmsConnected: Boolean = false,
    val statusMessage: String = "",
    val lastResponse: String = "",
    val dialogState: DialogState = DialogState(),
    val isOperationInProgress: Boolean = false,
    val serialNumber: String? = null
)

data class DialogState(
    val show: Boolean = false,
    val title: String = "",
    val message: String = "",
    val confirmText: String = "Confirm",
    val isDestructive: Boolean = false,
    val onConfirm: () -> Unit = {}
)

@HiltViewModel
class MeterConnectionViewModel @Inject constructor(
    private val bleRepository: BleRepository,
    private val dlmsRepository: DlmsRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow(MeterConnectionUiState())
    val uiState: StateFlow<MeterConnectionUiState> = _uiState.asStateFlow()

    private var currentReadJob: Job? = null
    private var connectionObserverJob: Job? = null
    private val operationMutex = Mutex()

    init {
        observeConnectionState()
    }

    private fun observeConnectionState() {
        connectionObserverJob?.cancel()
        connectionObserverJob = viewModelScope.launch {
            bleRepository.connectionState.collect { state ->
                _uiState.update { it.copy(connectionState = state) }

                // Reset DLMS state on disconnect
                if (state is BleConnectionState.Disconnected || state is BleConnectionState.Error) {
                    _uiState.update { it.copy(isDlmsConnected = false, serialNumber = null) }
                }
            }
        }
    }

    fun connectToDevice(device: BluetoothDevice) {
        viewModelScope.launch {
            _uiState.update { it.copy(selectedDevice = device) }
            bleRepository.connectToDevice(device)

            // Wait for BLE connection then establish DLMS
            bleRepository.connectionState.collect { state ->
                if (state is BleConnectionState.Connected) {
                    establishDlmsConnection()
                    return@collect
                }
                if (state is BleConnectionState.Error) {
                    return@collect
                }
            }
        }
    }

    private fun establishDlmsConnection() {
        viewModelScope.launch {
            dlmsRepository.establishConnection().collect { progress ->
                when (progress) {
                    is ConnectionProgress.Connecting -> {
                        _uiState.value = _uiState.value.copy(statusMessage = progress.message)
                    }
                    is ConnectionProgress.Connected -> {
                        _uiState.value = _uiState.value.copy(
                            statusMessage = "DLMS connected",
                            isDlmsConnected = true
                        )
                        fetchSerialNumber()
                    }
                    is ConnectionProgress.Failed -> {
                        _uiState.value = _uiState.value.copy(
                            connectionState = BleConnectionState.Error(progress.message)
                        )
                    }
                }
            }
        }
    }

    fun fetchSerialNumber() {
        viewModelScope.launch {
            dlmsRepository.readSerialNumber().collect { result ->
                if (result is DlmsRepository.MeterDataResult.Success && result.data.size > 1) {
                    // Index 0 is timestamp, index 1 is "Serial NO: XXXXXXXXXX"
                    val serialNo = result.data[1].replace("Serial NO: ", "")
                    _uiState.update { it.copy(serialNumber = serialNo) }
                }
            }
        }
    }

    fun requestDemandReset() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Demand Reset",
                    message = "Demand reset command will be issued.\n\n" +
                            "This will reset the maximum demand values stored in the meter.\n\n" +
                            "Are you sure you want to continue?",
                    confirmText = "Yes, Reset",
                    isDestructive = true,
                    onConfirm = {
                        executeDemandReset()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun dismissDialog() {
        _uiState.update { it.copy(dialogState = DialogState()) }
    }

    fun executeDemandReset() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Executing demand reset..."
            )}

            dlmsRepository.executeDemandReset().collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Loading -> {
                        _uiState.update { it.copy(lastResponse = "Sending demand reset command...") }
                    }
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Demand reset successful\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Demand reset failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    private fun getCurrentTimestamp(): String {
        return java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss", java.util.Locale.getDefault())
            .format(java.util.Date())
    }

    private fun readMeterData(objectIndex: Int, loadingMessage: String) {
        currentReadJob?.cancel()
        currentReadJob = viewModelScope.launch {
            _uiState.update { it.copy(isOperationInProgress = true) }

            operationMutex.withLock {
                dlmsRepository.readMeterData(objectIndex, 2).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Loading -> {
                            _uiState.update { it.copy(lastResponse = loadingMessage) }
                        }
                        is DlmsRepository.MeterDataResult.Partial -> {
                            _uiState.update { it.copy(lastResponse = result.data.joinToString("\n")) }
                        }
                        is DlmsRepository.MeterDataResult.Success -> {
                            _uiState.update {
                                it.copy(
                                    lastResponse = result.data.joinToString("\n"),
                                    isOperationInProgress = false
                                )
                            }
                        }
                        is DlmsRepository.MeterDataResult.Error -> {
                            _uiState.update {
                                it.copy(
                                    lastResponse = "Error: ${result.message}",
                                    isOperationInProgress = false
                                )
                            }
                        }
                    }
                }
            }
        }
    }

    fun readMeasure() = readMeterData(85, "Loading...")

    fun readState() = readMeterData(DLMS.IST_SPECIFICATION, "Loading state...")

    fun readBilling() = readMeterData(DLMS.IST_BILLING_PARAMS, "Loading billing data...")

    fun readEvent() = readMeterData(DLMS.IST_POWER_QUALITY, "Loading power quality events...")

    fun readLoad() = readMeterData(DLMS.IST_LOAD_PROFILE, "Loading load profile...")

    fun readAmpere() = readMeterData(DLMS.IST_AMPR_RECORD, "Loading current profile...")

    fun updateSettings(account: String, password: String, address: String, rank: String, scan: String, tick: String, interval: String) {
        dlmsRepository.updateSettings(account, password, address, rank, scan, tick, interval)
    }

    fun getSettings(): Map<String, String> {
        return dlmsRepository.getSettings()
    }

    fun disconnect() {
        viewModelScope.launch {
            bleRepository.disconnect()
            _uiState.value = MeterConnectionUiState()
        }
    }
}