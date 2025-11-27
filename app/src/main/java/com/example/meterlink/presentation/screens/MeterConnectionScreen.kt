package com.example.meterlink.presentation.screens

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import com.example.meterlink.data.repository.BleConnectionState
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@SuppressLint("MissingPermission")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MeterConnectionScreen(
    viewModel: MeterConnectionViewModel = hiltViewModel()
) {
    val uiState by viewModel.uiState.collectAsState()
    var showScanner by remember { mutableStateOf(false) }

    if (showScanner) {
        BleScannerScreen(
            onDeviceSelected = { device ->
                viewModel.connectToDevice(device)
                showScanner = false
            },
            onClose = { showScanner = false }
        )
    } else {
        Scaffold(
            topBar = {
                TopAppBar(title = { Text("MeterLink") })
            }
        ) { padding ->
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding)
                    .padding(16.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.Center
            ) {
                ConnectionStatusCard(uiState.connectionState)

                // Add after ConnectionStatusCard:
                if (uiState.statusMessage.isNotEmpty()) {
                    Text(
                        text = uiState.statusMessage,
                        style = MaterialTheme.typography.bodyMedium,
                        modifier = Modifier.padding(vertical = 8.dp)
                    )
                }

                if (uiState.lastResponse.isNotEmpty()) {
                    Card(modifier = Modifier.fillMaxWidth()) {
                        Text(
                            text = uiState.lastResponse,
                            modifier = Modifier.padding(16.dp)
                        )
                    }
                }

                Spacer(modifier = Modifier.height(24.dp))

                uiState.selectedDevice?.let { device ->
                    DeviceInfoCard(device)
                    Spacer(modifier = Modifier.height(24.dp))
                }

                when (uiState.connectionState) {
                    is BleConnectionState.Disconnected -> {
                        Button(
                            onClick = { showScanner = true },
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Text("Scan for Meters")
                        }
                    }
                    is BleConnectionState.Connected -> {
                        Column(modifier = Modifier.fillMaxWidth(), verticalArrangement = Arrangement.spacedBy(8.dp)) {
                            Button(
                                onClick = { /* Show operations screen */ },
                                modifier = Modifier.fillMaxWidth()
                            ) {
                                Text("Meter Operations")
                            }
                            Button(
                                onClick = { viewModel.disconnect() },
                                modifier = Modifier.fillMaxWidth(),
                                colors = ButtonDefaults.buttonColors(
                                    containerColor = MaterialTheme.colorScheme.error
                                )
                            ) {
                                Text("Disconnect")
                            }
                        }
                    }
                    is BleConnectionState.Connecting -> {
                        CircularProgressIndicator()
                    }
                    is BleConnectionState.Error -> {
                        Button(
                            onClick = { showScanner = true },
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Text("Retry")
                        }
                    }
                }
            }
        }
    }
}

@SuppressLint("MissingPermission")
@Composable
fun DeviceInfoCard(device: BluetoothDevice) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text("Device Name", style = MaterialTheme.typography.labelMedium)
            Text(device.name ?: "Unknown", style = MaterialTheme.typography.bodyLarge)
            Spacer(modifier = Modifier.height(8.dp))
            Text("MAC Address", style = MaterialTheme.typography.labelMedium)
            Text(device.address, style = MaterialTheme.typography.bodyMedium)
        }
    }
}

@Composable
fun ConnectionStatusCard(state: BleConnectionState) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = when (state) {
                is BleConnectionState.Connected -> MaterialTheme.colorScheme.primaryContainer
                is BleConnectionState.Error -> MaterialTheme.colorScheme.errorContainer
                else -> MaterialTheme.colorScheme.surfaceVariant
            }
        )
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text(
                text = when (state) {
                    is BleConnectionState.Disconnected -> "Disconnected"
                    is BleConnectionState.Connecting -> "Connecting..."
                    is BleConnectionState.Connected -> "Connected"
                    is BleConnectionState.Error -> "Error"
                },
                style = MaterialTheme.typography.headlineSmall
            )

            if (state is BleConnectionState.Error) {
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = state.message,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onErrorContainer
                )
            }
        }
    }
}