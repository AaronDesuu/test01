package com.example.meterlinkind.presentation.viewmodel

import android.bluetooth.BluetoothDevice
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.meterlinkind.data.model.AppAccounts
import com.example.meterlinkind.data.model.ClientSecurity
import com.example.meterlinkind.data.model.UserSession
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.data.repository.BleConnectionState
import com.example.meterlinkind.data.repository.BleRepository
import com.example.meterlinkind.data.repository.ConnectionProgress
import com.example.meterlinkind.data.repository.DlmsRepository
import com.example.meterlinkind.data.repository.SettingsRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class MeterConnectionUiState(
    /** DLMS client of the logged-in app user; null until someone logs in. */
    val client: ClientProfile? = null,
    val connectionState: BleConnectionState = BleConnectionState.Disconnected,
    val selectedDevice: BluetoothDevice? = null,
    /** Advertised name from the scan; `selectedDevice.name` goes null once the link closes. */
    val selectedDeviceName: String? = null,
    val isDlmsConnected: Boolean = false,
    val statusMessage: String = "",
    val serialNumber: String? = null
)

@HiltViewModel
class MeterConnectionViewModel @Inject constructor(
    private val bleRepository: BleRepository,
    private val dlmsRepository: DlmsRepository,
    private val settingsRepository: SettingsRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow(MeterConnectionUiState())
    val uiState: StateFlow<MeterConnectionUiState> = _uiState.asStateFlow()

    private var connectionObserverJob: Job? = null

    init {
        observeConnectionState()
        restoreSession()
    }

    /** Logs the saved user back in, unless the app went unused for [UserSession.TIMEOUT_MILLIS]. */
    private fun restoreSession() {
        val session = settingsRepository.getSession() ?: return
        val now = System.currentTimeMillis()
        if (session.isExpired(now)) {
            settingsRepository.clearSession()
        } else {
            settingsRepository.saveSession(session.copy(lastActiveMillis = now))
            _uiState.value = MeterConnectionUiState(client = session.client)
        }
    }

    /**
     * Call when the app comes to the foreground: logs out if it sat unused
     * for too long, otherwise counts as activity.
     */
    fun isTutorialSeen(): Boolean = settingsRepository.isTutorialSeen()

    fun setTutorialSeen() = settingsRepository.setTutorialSeen()

    fun getTutorialLanguage(): String = settingsRepository.getTutorialLanguage()

    fun setTutorialLanguage(code: String) = settingsRepository.setTutorialLanguage(code)

    fun onAppResumed() {
        val client = _uiState.value.client ?: return
        val session = settingsRepository.getSession()
        val now = System.currentTimeMillis()
        if (session == null || session.isExpired(now)) {
            logout()
        } else {
            settingsRepository.saveSession(UserSession(client, now))
        }
    }

    /** Call when the app goes to the background: the inactivity period starts now. */
    fun onAppPaused() {
        val client = _uiState.value.client ?: return
        settingsRepository.saveSession(UserSession(client, System.currentTimeMillis()))
    }

    private fun observeConnectionState() {
        connectionObserverJob?.cancel()
        connectionObserverJob = viewModelScope.launch {
            bleRepository.connectionState.collect { state ->
                _uiState.update { it.copy(connectionState = state) }

                // Reset DLMS state on disconnect
                if (state is BleConnectionState.Disconnected || state is BleConnectionState.Error) {
                    dlmsRepository.clearAssociation()
                    _uiState.update { it.copy(isDlmsConnected = false, serialNumber = null) }
                }
            }
        }
    }

    /** Opens the app as the DLMS client this login maps to; false if the login is wrong. */
    fun login(username: String, password: String): Boolean {
        val client = AppAccounts.clientFor(username, password) ?: return false
        settingsRepository.saveSession(UserSession(client, System.currentTimeMillis()))
        _uiState.value = MeterConnectionUiState(client = client)
        return true
    }

    fun logout() {
        settingsRepository.clearSession()
        viewModelScope.launch {
            dlmsRepository.release()
            bleRepository.disconnect()
            _uiState.value = MeterConnectionUiState()
        }
    }

    fun getClientSecurity(): ClientSecurity =
        _uiState.value.client?.let(settingsRepository::getClientSecurity) ?: ClientSecurity()

    /** Takes effect from the next association. */
    fun saveClientSecurity(security: ClientSecurity) {
        _uiState.value.client?.let { settingsRepository.saveClientSecurity(it, security) }
    }

    /** [name] comes from the scanner; a reconnect to the same device keeps the one we have. */
    fun connectToDevice(device: BluetoothDevice, name: String? = null) {
        viewModelScope.launch {
            _uiState.update {
                val known = it.selectedDeviceName.takeIf { _ -> it.selectedDevice?.address == device.address }
                it.copy(selectedDevice = device, selectedDeviceName = name ?: known)
            }
            bleRepository.connectToDevice(device)

            val terminal = bleRepository.connectionState.first {
                it is BleConnectionState.Connected || it is BleConnectionState.Error
            }
            if (terminal is BleConnectionState.Connected) {
                establishDlmsConnection()
            }
        }
    }

    private fun establishDlmsConnection() {
        val client = _uiState.value.client ?: return
        viewModelScope.launch {
            dlmsRepository.establishConnection(client, settingsRepository.getClientSecurity(client)).collect { progress ->
                when (progress) {
                    is ConnectionProgress.Connecting -> {
                        _uiState.value = _uiState.value.copy(statusMessage = progress.message)
                    }
                    is ConnectionProgress.Connected -> {
                        _uiState.value = _uiState.value.copy(
                            statusMessage = "Associated as ${client.label}",
                            isDlmsConnected = true
                        )
                        fetchSerialNumber()
                    }
                    is ConnectionProgress.Failed -> {
                        // Close the GATT too, so "Reconnect" starts from a clean link.
                        bleRepository.failAndDisconnect(progress.message)
                    }
                }
            }
        }
    }

    fun fetchSerialNumber() {
        viewModelScope.launch {
            dlmsRepository.readSerialNumber().collect { result ->
                if (result is DlmsRepository.MeterDataResult.Success && result.data.isNotEmpty()) {
                    _uiState.update { it.copy(serialNumber = result.data[0]) }
                }
            }
        }
    }


    fun disconnect() {
        viewModelScope.launch {
            dlmsRepository.release()
            bleRepository.disconnect()
            _uiState.value = MeterConnectionUiState(client = _uiState.value.client)
        }
    }
}