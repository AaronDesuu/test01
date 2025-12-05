package com.example.meterlink.presentation.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.meterlink.data.repository.DlmsRepository
import com.example.meterlink.dlms.DLMS
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class MaintenanceUiState(
    val isOperationInProgress: Boolean = false,
    val lastResponse: String = "",
    val dialogState: DialogState = DialogState()
)

@HiltViewModel
class MaintenanceViewModel @Inject constructor(
    private val dlmsRepository: DlmsRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow(MaintenanceUiState())
    val uiState: StateFlow<MaintenanceUiState> = _uiState.asStateFlow()

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

    fun dismissDialog() {
        _uiState.update { it.copy(dialogState = DialogState()) }
    }

    private fun getCurrentTimestamp(): String {
        return java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss", java.util.Locale.getDefault())
            .format(java.util.Date())
    }
}