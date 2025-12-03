package com.example.meterlink.presentation.viewmodel

import android.bluetooth.BluetoothDevice
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.meterlink.data.protocol.DlmsResult
import com.example.meterlink.data.repository.BleConnectionState
import com.example.meterlink.data.repository.BleRepository
import com.example.meterlink.data.repository.ConnectionProgress
import com.example.meterlink.data.repository.DlmsRepository
import com.example.meterlink.dlms.DLMS
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class MeterConnectionUiState(
    val connectionState: BleConnectionState = BleConnectionState.Disconnected,
    val selectedDevice: BluetoothDevice? = null,
    val isDlmsConnected: Boolean = false,
    val statusMessage: String = "",
    val lastResponse: String = ""
)

@HiltViewModel
class MeterConnectionViewModel @Inject constructor(
    private val bleRepository: BleRepository,
    private val dlmsRepository: DlmsRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow(MeterConnectionUiState())
    val uiState: StateFlow<MeterConnectionUiState> = _uiState.asStateFlow()

    fun connectToDevice(device: BluetoothDevice) {
        viewModelScope.launch {
            _uiState.value = _uiState.value.copy(selectedDevice = device)

            bleRepository.connectToDevice(device).collect { state ->
                _uiState.value = _uiState.value.copy(connectionState = state)

                if (state is BleConnectionState.Connected) {
                    establishDlmsConnection(device)
                }
            }
        }
    }

    private fun establishDlmsConnection(device: BluetoothDevice) {
        viewModelScope.launch {
            dlmsRepository.establishConnection(device).collect { progress ->
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
        viewModelScope.launch {
            dlmsRepository.readMeterData(85, 2).collect { result ->  // Changed from 24 to 85
                when (result) {
                    is DlmsRepository.MeterDataResult.Loading -> {
                        _uiState.update { it.copy(lastResponse = "Loading...") }
                    }
                    is DlmsRepository.MeterDataResult.Success -> {
                        _uiState.update { it.copy(lastResponse = result.data.joinToString("\n")) }
                    }
                    is DlmsRepository.MeterDataResult.Error -> {
                        _uiState.update { it.copy(lastResponse = "Error: ${result.message}") }
                    }
                    else -> {}
                }
            }
        }
    }

    fun readState() {
        viewModelScope.launch {
            val stateData = mutableListOf<String>()

            // Read sequentially with proper error handling
            try {
                dlmsRepository.readMeterData(DLMS.IST_SERIAL_NO).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Success -> {
                            stateData.add("Serial: ${result.data.joinToString()}")
                        }
                        is DlmsRepository.MeterDataResult.Error -> {
                            stateData.add("Serial: Error")
                        }
                        else -> {}
                    }
                }

                dlmsRepository.readMeterData(DLMS.IST_APPROVAL_NO).collect { result ->
                    when (result) {
                        is DlmsRepository.MeterDataResult.Success -> {
                            stateData.add("Approval: ${result.data.joinToString()}")
                        }
                        is DlmsRepository.MeterDataResult.Error -> {
                            stateData.add("Approval: Error")
                        }
                        else -> {}
                    }
                }

                _uiState.value = _uiState.value.copy(
                    lastResponse = stateData.joinToString("\n")
                )
            } catch (e: Exception) {
                _uiState.value = _uiState.value.copy(
                    lastResponse = "Error reading state: ${e.message}"
                )
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