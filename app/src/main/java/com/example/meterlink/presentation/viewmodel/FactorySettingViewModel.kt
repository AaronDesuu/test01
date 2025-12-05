package com.example.meterlink.presentation.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.meterlink.data.repository.DlmsRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class FactorySettingUiState(
    val isOperationInProgress: Boolean = false,
    val lastResponse: String = "",
    val dialogState: DialogState = DialogState()
)

@HiltViewModel
class FactorySettingViewModel @Inject constructor(
    private val dlmsRepository: DlmsRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow(FactorySettingUiState())
    val uiState: StateFlow<FactorySettingUiState> = _uiState.asStateFlow()

    // Set Meter Type
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

    // Whole Clear
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

    // Missing Neutral Detection
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

    // Power Network Configuration
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

    // Potential Configuration
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

    // Type Set Configuration
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

    fun dismissDialog() {
        _uiState.update { it.copy(dialogState = DialogState()) }
    }

    private fun getCurrentTimestamp(): String {
        return java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss", java.util.Locale.getDefault())
            .format(java.util.Date())
    }
}