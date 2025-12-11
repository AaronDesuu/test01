package com.example.meterlink.presentation.screens

import android.annotation.SuppressLint
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.foundation.background
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.automirrored.filled.BluetoothSearching
import androidx.compose.material.icons.filled.Bluetooth
import androidx.compose.material.icons.filled.BluetoothDisabled
import androidx.compose.material.icons.filled.Build
import androidx.compose.material.icons.filled.CheckCircle
import androidx.compose.material.icons.filled.Factory
import androidx.compose.material.icons.filled.PowerOff
import androidx.compose.material.icons.filled.Settings
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import androidx.compose.foundation.combinedClickable
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.ui.hapticfeedback.HapticFeedbackType
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalHapticFeedback
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.SnackbarDuration
import kotlinx.coroutines.launch
import androidx.hilt.lifecycle.viewmodel.compose.hiltViewModel
import com.example.meterlink.data.repository.BleConnectionState
import com.example.meterlink.presentation.components.AppDrawer
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MeterConnectionScreen(
    viewModel: MeterConnectionViewModel = hiltViewModel()
) {
    val drawerState = rememberDrawerState(DrawerValue.Closed)
    val scope = rememberCoroutineScope()
    var currentScreen by rememberSaveable { mutableStateOf("home") }

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
                closeDrawer = { scope.launch { drawerState.close() } },
                connectionViewModel = viewModel
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
                    onNavigateToMaintenance = { currentScreen = "maintenance" },
                    onNavigateToFactory = { currentScreen = "factory" },
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

@SuppressLint("MissingPermission")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeContent(
    modifier: Modifier = Modifier,
    viewModel: MeterConnectionViewModel,
    onNavigateToOperations: () -> Unit = {},
    onNavigateToMaintenance: () -> Unit = {},
    onNavigateToFactory: () -> Unit = {},
) {
    val uiState by viewModel.uiState.collectAsState()
    var showScanner by remember { mutableStateOf(false) }
    val snackbarHostState = remember { SnackbarHostState() }
    val scope = rememberCoroutineScope()

    if (showScanner) {
        BleScannerScreen(
            onDeviceSelected = { device ->
                viewModel.connectToDevice(device)
                showScanner = false
            },
            onClose = { showScanner = false }
        )
    } else {
        Box(
            modifier = modifier.fillMaxSize()
        ) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                Column(
                    modifier = Modifier
                        .widthIn(max = 500.dp)
                        .padding(16.dp)
                        .verticalScroll(rememberScrollState()),
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    // Hero Status Card
                    ElevatedCard(
                        modifier = Modifier.fillMaxWidth(),
                        elevation = CardDefaults.elevatedCardElevation(defaultElevation = 8.dp)
                    ) {
                        Column(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(24.dp),
                            horizontalAlignment = Alignment.CenterHorizontally
                        ) {
                            // Status Icon
                            Box(
                                modifier = Modifier
                                    .size(80.dp)
                                    .background(
                                        color = when (uiState.connectionState) {
                                            is BleConnectionState.Connected -> MaterialTheme.colorScheme.primaryContainer
                                            is BleConnectionState.Connecting -> MaterialTheme.colorScheme.secondaryContainer
                                            else -> MaterialTheme.colorScheme.surfaceVariant
                                        },
                                        shape = CircleShape
                                    ),
                                contentAlignment = Alignment.Center
                            ) {
                                Icon(
                                    imageVector = when (uiState.connectionState) {
                                        is BleConnectionState.Connected -> Icons.Default.CheckCircle
                                        is BleConnectionState.Connecting -> Icons.AutoMirrored.Filled.BluetoothSearching
                                        else -> Icons.Default.BluetoothDisabled
                                    },
                                    contentDescription = null,
                                    modifier = Modifier.size(40.dp),
                                    tint = when (uiState.connectionState) {
                                        is BleConnectionState.Connected -> MaterialTheme.colorScheme.primary
                                        else -> MaterialTheme.colorScheme.onSurfaceVariant
                                    }
                                )
                            }

                            Spacer(modifier = Modifier.height(16.dp))

                            Text(
                                text = when (uiState.connectionState) {
                                    is BleConnectionState.Connected -> "Connected"
                                    is BleConnectionState.Connecting -> "Connecting..."
                                    is BleConnectionState.Error -> "Connection Failed"
                                    else -> "Not Connected"
                                },
                                style = MaterialTheme.typography.headlineSmall,
                                fontWeight = FontWeight.Bold
                            )

                            uiState.selectedDevice?.let { device ->
                                Spacer(modifier = Modifier.height(8.dp))
                                CopyableText(
                                    text = device.name ?: "Unknown Device",
                                    style = MaterialTheme.typography.titleMedium,
                                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                                    onCopied = { text ->
                                        scope.launch {
                                            snackbarHostState.showSnackbar(
                                                message = "Copied: $text",
                                                duration = SnackbarDuration.Short
                                            )
                                        }
                                    }
                                )

                                Spacer(modifier = Modifier.height(12.dp))

                                // Serial Number and MAC Address
                                Column(
                                    modifier = Modifier.fillMaxWidth(),
                                    horizontalAlignment = Alignment.CenterHorizontally,
                                    verticalArrangement = Arrangement.spacedBy(4.dp)
                                ) {
                                    uiState.serialNumber?.let {
                                        if (it.isNotEmpty()) {
                                            Row(
                                                horizontalArrangement = Arrangement.Center,
                                                verticalAlignment = Alignment.CenterVertically,
                                                modifier = Modifier.fillMaxWidth()
                                            ) {
                                                Text(
                                                    text = "S/N: ",
                                                    style = MaterialTheme.typography.bodySmall,
                                                    color = MaterialTheme.colorScheme.onSurfaceVariant
                                                )
                                                CopyableText(
                                                    text = uiState.serialNumber!!,
                                                    style = MaterialTheme.typography.bodySmall,
                                                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                                                    fontWeight = FontWeight.SemiBold,
                                                    onCopied = { text ->
                                                        scope.launch {
                                                            snackbarHostState.showSnackbar(
                                                                message = "Copied: $text",
                                                                duration = SnackbarDuration.Short
                                                            )
                                                        }
                                                    }
                                                )
                                                if (uiState.serialNumber ==  "0000000000") {
                                                    Spacer(Modifier.width(8.dp))
                                                    IconButton(
                                                        onClick = { viewModel.fetchSerialNumber() },
                                                        modifier = Modifier.size(24.dp)
                                                    ) {
                                                        Icon(
                                                            Icons.Default.Refresh,
                                                            contentDescription = "Refresh Serial Number",
                                                            modifier = Modifier.size(16.dp),
                                                            tint = MaterialTheme.colorScheme.primary
                                                        )
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    CopyableText(
                                        text = device.address,
                                        style = MaterialTheme.typography.bodySmall,
                                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                                        onCopied = { text ->
                                            scope.launch {
                                                snackbarHostState.showSnackbar(
                                                    message = "Copied: $text",
                                                    duration = SnackbarDuration.Short
                                                )
                                            }
                                        }
                                    )
                                }
                            }
                        }
                    }

                    when (uiState.connectionState) {
                        is BleConnectionState.Disconnected, is BleConnectionState.Error -> {
                            Column(
                                modifier = Modifier.fillMaxWidth(),
                                verticalArrangement = Arrangement.spacedBy(12.dp)
                            ) {
                                // Show reconnect button if a device was previously selected
                                uiState.selectedDevice?.let { device ->
                                    Button(
                                        onClick = { viewModel.connectToDevice(device) },
                                        modifier = Modifier
                                            .fillMaxWidth()
                                            .height(56.dp)
                                    ) {
                                        Icon(Icons.AutoMirrored.Filled.BluetoothSearching, contentDescription = null)
                                        Spacer(Modifier.width(8.dp))
                                        Text("Reconnect to ${device.name ?: "Device"}")
                                    }
                                }

                                Button(
                                    onClick = { showScanner = true },
                                    modifier = Modifier
                                        .fillMaxWidth()
                                        .height(56.dp),
                                    colors = if (uiState.selectedDevice != null) {
                                        ButtonDefaults.outlinedButtonColors()
                                    } else {
                                        ButtonDefaults.buttonColors()
                                    }
                                ) {
                                    Icon(Icons.Default.Bluetooth, contentDescription = null)
                                    Spacer(Modifier.width(8.dp))
                                    Text("Scan for Meters")
                                }
                            }
                        }

                        is BleConnectionState.Connected -> {
                            Text(
                                text = "Quick Actions",
                                style = MaterialTheme.typography.titleMedium,
                                fontWeight = FontWeight.SemiBold
                            )

                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                horizontalArrangement = Arrangement.spacedBy(12.dp)
                            ) {
                                QuickActionCard(
                                    title = "Operations",
                                    icon = Icons.Default.Settings,
                                    onClick = onNavigateToOperations,
                                    modifier = Modifier.weight(1f)
                                )
                                QuickActionCard(
                                    title = "Maintenance",
                                    icon = Icons.Default.Build,
                                    onClick = onNavigateToMaintenance,
                                    modifier = Modifier.weight(1f)
                                )
                            }

                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                horizontalArrangement = Arrangement.spacedBy(12.dp)
                            ) {
                                QuickActionCard(
                                    title = "Factory",
                                    icon = Icons.Default.Factory,
                                    onClick = onNavigateToFactory,
                                    modifier = Modifier.weight(1f)
                                )
                                QuickActionCard(
                                    title = "Disconnect",
                                    icon = Icons.Default.PowerOff,
                                    onClick = { viewModel.disconnect() },
                                    modifier = Modifier.weight(1f),
                                    containerColor = MaterialTheme.colorScheme.errorContainer,
                                    contentColor = MaterialTheme.colorScheme.onErrorContainer
                                )
                            }
                        }

                        is BleConnectionState.Connecting -> {
                            Box(
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .padding(32.dp),
                                contentAlignment = Alignment.Center
                            ) {
                                CircularProgressIndicator()
                            }
                        }
                    }
                }
            }

            // Snackbar at bottom
            SnackbarHost(
                hostState = snackbarHostState,
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .padding(16.dp),
                snackbar = { data ->
                    Snackbar(
                        snackbarData = data,
                        containerColor = MaterialTheme.colorScheme.primaryContainer,
                        contentColor = MaterialTheme.colorScheme.onPrimaryContainer,
                        shape = RoundedCornerShape(12.dp)
                    )
                }
            )
        }
    }
}

@Composable
fun QuickActionCard(
    title: String,
    icon: ImageVector,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    containerColor: Color = MaterialTheme.colorScheme.primaryContainer,
    contentColor: Color = MaterialTheme.colorScheme.onPrimaryContainer
) {
    ElevatedCard(
        onClick = onClick,
        modifier = modifier.height(120.dp),
        colors = CardDefaults.elevatedCardColors(
            containerColor = containerColor
        )
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Icon(
                imageVector = icon,
                contentDescription = null,
                modifier = Modifier.size(32.dp),
                tint = contentColor
            )
            Spacer(Modifier.height(8.dp))
            Text(
                text = title,
                style = MaterialTheme.typography.titleSmall,
                fontWeight = FontWeight.Medium,
                color = contentColor
            )
        }
    }
}

@Composable
fun CopyableText(
    text: String,
    displayText: String = text,
    style: androidx.compose.ui.text.TextStyle = MaterialTheme.typography.bodySmall,
    color: Color = MaterialTheme.colorScheme.onSurfaceVariant,
    fontWeight: FontWeight? = null,
    onCopied: (String) -> Unit
) {
    val context = LocalContext.current
    val clipboardManager = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
    val hapticFeedback = LocalHapticFeedback.current

    Text(
        text = displayText,
        style = style,
        color = color,
        fontWeight = fontWeight,
        modifier = Modifier.combinedClickable(
            onClick = { },
            onLongClick = {
                hapticFeedback.performHapticFeedback(HapticFeedbackType.LongPress)
                val clip = ClipData.newPlainText("Copied Text", text)
                clipboardManager.setPrimaryClip(clip)
                onCopied(text)
            }
        )
    )
}