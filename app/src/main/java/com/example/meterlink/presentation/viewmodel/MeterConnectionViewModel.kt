package com.example.meterlink.presentation.viewmodel

import android.bluetooth.BluetoothDevice
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.meterlink.data.repository.BleConnectionState
import com.example.meterlink.data.repository.BleRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import javax.inject.Inject

data class MeterConnectionUiState(
    val connectionState: BleConnectionState = BleConnectionState.Disconnected,
    val selectedDevice: BluetoothDevice? = null,
    val isScanning: Boolean = false
)

@HiltViewModel
class MeterConnectionViewModel @Inject constructor(
    private val bleRepository: BleRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow(MeterConnectionUiState())
    val uiState: StateFlow<MeterConnectionUiState> = _uiState.asStateFlow()

    fun connectToDevice(device: BluetoothDevice) {
        viewModelScope.launch {
            _uiState.value = _uiState.value.copy(selectedDevice = device)
            bleRepository.connectToDevice(device).collect { state ->
                _uiState.value = _uiState.value.copy(connectionState = state)
            }
        }
    }

    fun disconnect() {
        bleRepository.disconnect()
        _uiState.value = _uiState.value.copy(
            connectionState = BleConnectionState.Disconnected,
            selectedDevice = null
        )
    }

    override fun onCleared() {
        super.onCleared()
        bleRepository.disconnect()
    }
}