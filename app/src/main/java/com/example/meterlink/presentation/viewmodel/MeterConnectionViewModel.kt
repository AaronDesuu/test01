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
    val isOperationInProgress: Boolean = false
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
    private val operationMutex = Mutex()

    fun connectToDevice(device: BluetoothDevice) {
        viewModelScope.launch {
            _uiState.value = _uiState.value.copy(selectedDevice = device)

            bleRepository.connectToDevice(device).collect { state ->
                _uiState.value = _uiState.value.copy(connectionState = state)

                if (state is BleConnectionState.Connected) {
                    establishDlmsConnection()
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

    fun readMeasure() {
        currentReadJob?.cancel()
        currentReadJob = viewModelScope.launch {
            _uiState.update { it.copy(isOperationInProgress = true) }

            operationMutex.withLock {
                dlmsRepository.readMeterData(85, 2).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Loading -> {
                            _uiState.update { it.copy(lastResponse = "Loading...") }
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

    fun readState() {
        currentReadJob?.cancel()
        currentReadJob = viewModelScope.launch {
            _uiState.update { it.copy(isOperationInProgress = true) }

            operationMutex.withLock {
                dlmsRepository.readMeterData(DLMS.IST_SPECIFICATION, 2).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Loading -> {
                            _uiState.update { it.copy(lastResponse = "Loading state...") }
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
                    onConfirm = { executeDemandReset() }
                )
            )
        }
    }

    fun dismissDialog() {
        _uiState.update { it.copy(dialogState = DialogState()) }
    }

    private fun executeDemandReset() {
        viewModelScope.launch {
            _uiState.update {
                it.copy(
                    dialogState = DialogState(),
                    lastResponse = "Executing demand reset...",
                    isOperationInProgress = true
                )
            }

            dlmsRepository.executeAction(DLMS.IST_DEMAND_RESET, 1, "0f00").collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Loading -> {
                        _uiState.update { it.copy(lastResponse = "Resetting demand...") }
                    }
                    is DlmsRepository.MeterDataResult.Partial -> {
                        _uiState.update {
                            it.copy(lastResponse = "Demand reset partial\n${result.data.joinToString("\n")}")
                        }
                    }
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update {
                            it.copy(
                                lastResponse = "Demand reset successful\n${result.data.joinToString("\n")}",
                                isOperationInProgress = false
                            )
                        }
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update {
                            it.copy(
                                lastResponse = "Demand reset failed: ${result.message}",
                                isOperationInProgress = false
                            )
                        }
                    }
                }
            }
        }
    }

    fun readBilling() {
        currentReadJob?.cancel()
        currentReadJob = viewModelScope.launch {
            _uiState.update { it.copy(isOperationInProgress = true) }

            operationMutex.withLock {
                dlmsRepository.readMeterData(DLMS.IST_BILLING_PARAMS, 2).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Loading -> {
                            _uiState.update { it.copy(lastResponse = "Loading billing data...") }
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

    fun readEvent() {
        currentReadJob?.cancel()
        currentReadJob = viewModelScope.launch {
            _uiState.update { it.copy(isOperationInProgress = true) }

            operationMutex.withLock {
                dlmsRepository.readMeterData(DLMS.IST_POWER_QUALITY, 2).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Loading -> {
                            _uiState.update { it.copy(lastResponse = "Loading power quality events...") }
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

    fun readLoad() {
        currentReadJob?.cancel()
        currentReadJob = viewModelScope.launch {
            _uiState.update { it.copy(isOperationInProgress = true) }

            operationMutex.withLock {
                dlmsRepository.readMeterData(DLMS.IST_LOAD_PROFILE, 2).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Loading -> {
                            _uiState.update { it.copy(lastResponse = "Loading load profile...") }
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

    fun readAmpere() {
        currentReadJob?.cancel()
        currentReadJob = viewModelScope.launch {
            _uiState.update { it.copy(isOperationInProgress = true) }

            operationMutex.withLock {
                dlmsRepository.readMeterData(DLMS.IST_AMPR_RECORD, 2).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Loading -> {
                            _uiState.update { it.copy(lastResponse = "Loading current profile...") }
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