package com.example.meterlink.presentation.screens

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.hilt.lifecycle.viewmodel.compose.hiltViewModel
import com.example.meterlink.data.repository.BleConnectionState
import com.example.meterlink.presentation.components.AppDrawer
import com.example.meterlink.presentation.components.ConnectionStatusCard
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel
import kotlinx.coroutines.launch

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MeterConnectionScreen(
    viewModel: MeterConnectionViewModel = hiltViewModel()
) {
    val drawerState = rememberDrawerState(DrawerValue.Closed)
    val scope = rememberCoroutineScope()
    var currentScreen by remember { mutableStateOf("home") }

    ModalNavigationDrawer(
        drawerState = drawerState,
        drawerContent = {
            AppDrawer(
                currentRoute = currentScreen,
                navigateToHome = { currentScreen = "home" },
                navigateToOperations = { currentScreen = "operations" },
                navigateToMaintenance = { currentScreen = "maintenance" },
                navigateToFactory = { currentScreen = "factory" },
                navigateToSettings = { currentScreen = "settings" },
                closeDrawer = { scope.launch { drawerState.close() } }
            )
        }
    ) {
        Scaffold(
            topBar = {
                TopAppBar(
                    title = { Text(getScreenTitle(currentScreen)) },
                    navigationIcon = {
                        IconButton(onClick = { scope.launch { drawerState.open() } }) {
                            Icon(Icons.Default.Menu, "Menu")
                        }
                    }
                )
            }
        ) { padding ->
            when (currentScreen) {
                "home" -> HomeContent(
                    viewModel = viewModel,
                    onNavigateToOperations = { currentScreen = "operations" },
                    modifier = Modifier.padding(padding)
                )
                "operations" -> MeterOperationsScreen(
                    viewModel = viewModel,
                    onNavigateHome = { currentScreen = "home" },
                    modifier = Modifier.padding(padding)
                )
                "maintenance" -> MaintenanceScreen(
                    connectionViewModel = viewModel,
                    maintenanceViewModel = hiltViewModel(),
                    onNavigateHome = { currentScreen = "home" },
                    modifier = Modifier.padding(padding)
                )
                "factory" -> FactorySettingScreen(
                    connectionViewModel = viewModel,
                    factoryViewModel = hiltViewModel(),
                    onNavigateHome = { currentScreen = "home" },
                    modifier = Modifier.padding(padding)
                )
                "settings" -> SettingsScreen(
                    viewModel = viewModel,
                    onBack = { currentScreen = "home" },
                    modifier = Modifier.padding(padding)
                )
            }
        }
    }
}

private fun getScreenTitle(screen: String): String = when (screen) {
    "home" -> "Meter Link"
    "operations" -> "Operations"
    "maintenance" -> "Maintenance"
    "settings" -> "Settings"
    "factory" -> "Factory Setting"
    else -> "Meter Link"
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeContent(
    modifier: Modifier = Modifier,
    viewModel: MeterConnectionViewModel,
    onNavigateToOperations: () -> Unit = {},
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
        Column(
            modifier = modifier
                .fillMaxSize()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            ConnectionStatusCard(uiState.connectionState)

            if (uiState.statusMessage.isNotEmpty()) {
                Text(
                    text = uiState.statusMessage,
                    style = MaterialTheme.typography.bodyMedium,
                    modifier = Modifier.padding(vertical = 8.dp)
                )
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
                    Column(
                        modifier = Modifier.fillMaxWidth(),
                        verticalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        Button(
                            onClick = { onNavigateToOperations() },
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