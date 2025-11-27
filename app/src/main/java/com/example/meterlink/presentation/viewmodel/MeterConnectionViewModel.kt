package com.example.meterlink.presentation.viewmodel

import android.bluetooth.BluetoothDevice
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.meterlink.data.protocol.DlmsResult
import com.example.meterlink.data.repository.BleConnectionState
import com.example.meterlink.data.repository.BleRepository
import com.example.meterlink.data.repository.ConnectionProgress
import com.example.meterlink.data.repository.DlmsRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
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

    fun disconnect() {
        viewModelScope.launch {
            bleRepository.disconnect()
            _uiState.value = MeterConnectionUiState()
        }
    }
}