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

    private fun fetchSerialNumber() {
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

    // Set Clock
    fun requestSetClock() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Set Clock",
                    message = "Set meter clock to current system time?",
                    confirmText = "Set Clock",
                    isDestructive = false,
                    onConfirm = {
                        executeSetClock()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeSetClock() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Setting meter clock..."
            )}

            dlmsRepository.setDateTime().collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Clock set successfully\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed to set clock: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    // Reset Billing
    fun requestResetBilling() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Clear Billing Data",
                    message = "⚠️ This will clear all billing data. This cannot be undone!",
                    confirmText = "Clear Billing",
                    isDestructive = true,
                    onConfirm = {
                        executeResetBilling()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeResetBilling() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Clearing billing data..."
            )}

            dlmsRepository.executeAction(DLMS.IST_BILLING_PARAMS, 1, "0f00").collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Billing data cleared\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    // Reset Event
    fun requestResetEvent() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Clear Event Data",
                    message = "⚠️ This will clear all event/power quality data. This cannot be undone!",
                    confirmText = "Clear Events",
                    isDestructive = true,
                    onConfirm = {
                        executeResetEvent()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeResetEvent() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Clearing event data..."
            )}

            dlmsRepository.executeAction(DLMS.IST_POWER_QUALITY, 1, "0f00").collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Event data cleared\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    // Reset Load
    fun requestResetLoad() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Clear Load Profile",
                    message = "⚠️ This will clear all load profile data. This cannot be undone!",
                    confirmText = "Clear Load",
                    isDestructive = true,
                    onConfirm = {
                        executeResetLoad()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeResetLoad() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Clearing load profile..."
            )}

            dlmsRepository.executeAction(DLMS.IST_LOAD_PROFILE, 1, "0f00").collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Load profile cleared\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    // Reset Ampere
    fun requestResetAmpere() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Clear Current Profile",
                    message = "⚠️ This will clear all ampere/current profile data. This cannot be undone!",
                    confirmText = "Clear Current",
                    isDestructive = true,
                    onConfirm = {
                        executeResetAmpere()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeResetAmpere() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Clearing current profile..."
            )}

            dlmsRepository.executeAction(DLMS.IST_AMPR_RECORD, 1, "0f00").collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Current profile cleared\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }
    // Factory Settings Methods

    fun requestSetMeterType() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Set Meter Type",
                    message = "Configure meter type for Imp-Exp/ABS/NET?",
                    confirmText = "Configure",
                    isDestructive = false,
                    onConfirm = {
                        executeSetMeterType()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeSetMeterType() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Configuring meter type..."
            )}

            dlmsRepository.setMeterType().collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Meter type configured\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    fun requestWholeClear() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Whole Clear",
                    message = "⚠️ This will clear ALL measured data. This cannot be undone!",
                    confirmText = "Clear All",
                    isDestructive = true,
                    onConfirm = {
                        executeWholeClear()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeWholeClear() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Clearing all measured data..."
            )}

            dlmsRepository.wholeClearMeasuredData().collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ All measured data cleared\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    fun requestMissingNeutralOn() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Enable Missing Neutral Detection",
                    message = "Enable missing neutral detection?",
                    confirmText = "Enable",
                    isDestructive = false,
                    onConfirm = {
                        executeMissingNeutralOn()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeMissingNeutralOn() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Enabling missing neutral detection..."
            )}

            dlmsRepository.setMissingNeutral(true).collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Missing neutral enabled\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    fun requestMissingNeutralOff() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Disable Missing Neutral Detection",
                    message = "Disable missing neutral detection?",
                    confirmText = "Disable",
                    isDestructive = false,
                    onConfirm = {
                        executeMissingNeutralOff()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeMissingNeutralOff() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Disabling missing neutral detection..."
            )}

            dlmsRepository.setMissingNeutral(false).collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Missing neutral disabled\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    fun requestPowerNetworkOne() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Power Network ONE",
                    message = "Set power network configuration to ONE?",
                    confirmText = "Set",
                    isDestructive = false,
                    onConfirm = {
                        executePowerNetworkOne()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executePowerNetworkOne() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Setting power network to ONE..."
            )}

            dlmsRepository.setPowerNetwork(1).collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Power network set to ONE\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    fun requestPowerNetworkTwo() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Power Network TWO",
                    message = "Set power network configuration to TWO?",
                    confirmText = "Set",
                    isDestructive = false,
                    onConfirm = {
                        executePowerNetworkTwo()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executePowerNetworkTwo() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Setting power network to TWO..."
            )}

            dlmsRepository.setPowerNetwork(2).collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Power network set to TWO\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    fun requestPotential() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Set Potential",
                    message = "Configure potential settings?",
                    confirmText = "Configure",
                    isDestructive = false,
                    onConfirm = {
                        executePotential()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executePotential() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Configuring potential..."
            )}

            dlmsRepository.setPotential().collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Potential configured\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
                }
            }
        }
    }

    fun requestTypeSet() {
        _uiState.update {
            it.copy(
                dialogState = DialogState(
                    show = true,
                    title = "Type Set",
                    message = "Configure type settings?",
                    confirmText = "Configure",
                    isDestructive = false,
                    onConfirm = {
                        executeTypeSet()
                        dismissDialog()
                    }
                )
            )
        }
    }

    fun executeTypeSet() {
        viewModelScope.launch {
            _uiState.update { it.copy(
                isOperationInProgress = true,
                lastResponse = "Configuring type set..."
            )}

            dlmsRepository.setTypeSet().collect { result ->
                when (result) {
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(
                            lastResponse = "✓ Type set configured\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(
                            lastResponse = "✗ Failed: ${result.message}\n${getCurrentTimestamp()}",
                            isOperationInProgress = false
                        )}
                    }
                    else -> {}
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