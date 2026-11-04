package com.example.meterlinkind.presentation.screens

import android.annotation.SuppressLint
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.activity.compose.BackHandler
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
import androidx.compose.material.icons.filled.CheckCircle
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Schedule
import androidx.compose.material.icons.filled.Speed
import androidx.compose.material.icons.filled.TableChart
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
import com.example.meterlinkind.data.repository.BleConnectionState
import com.example.meterlinkind.presentation.components.AppDrawer
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
import com.example.meterlinkind.presentation.viewmodel.ObjectBrowserViewModel
import com.example.meterlinkind.data.protocol.InterfaceClass

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MeterConnectionScreen(
    viewModel: MeterConnectionViewModel = hiltViewModel()
) {
    val drawerState = rememberDrawerState(DrawerValue.Closed)
    val scope = rememberCoroutineScope()
    var currentScreen by rememberSaveable { mutableStateOf("home") }
    var keysFromSettings by rememberSaveable { mutableStateOf(false) }
    val connection by viewModel.uiState.collectAsState()
    val loggedInClient = connection.client

    if (loggedInClient == null) {
        // Show the keys first, so they can be checked before connecting.
        LoginScreen(onLogin = { user, pass ->
            viewModel.login(user, pass).also {
                if (it) {
                    keysFromSettings = false
                    currentScreen = "keys"
                }
            }
        })
        return
    }

    val browserViewModel: ObjectBrowserViewModel = hiltViewModel()
    // OBIS of the object open in a browser screen; null shows that class's list.
    var openObis by rememberSaveable { mutableStateOf<String?>(null) }
    val openObject = openObis?.let(browserViewModel::find)
    val navigate = { route: String -> currentScreen = route; openObis = null }

    BackHandler(enabled = openObject != null) { openObis = null }

    // Sub-pages of Settings go back to Settings; Security Keys right after login goes on to Home.
    val parentRoute = SUB_PAGES[currentScreen]?.takeIf { currentScreen != "keys" || keysFromSettings }
    BackHandler(enabled = openObject == null && parentRoute != null) { navigate(parentRoute!!) }

    var showScanner by rememberSaveable { mutableStateOf(false) }
    // Declared last, so Back closes the scanner before anything else.
    BackHandler(enabled = showScanner) { showScanner = false }
    // From any screen: reconnect to the last meter, or scan for one.
    val connect = {
        val device = connection.selectedDevice
        if (device != null) viewModel.connectToDevice(device) else showScanner = true
    }

    // The object list belongs to a meter (serial) and client; load the stored one when either changes.
    LaunchedEffect(connection.serialNumber, loggedInClient) {
        browserViewModel.onMeter(connection.serialNumber, loggedInClient)
    }

    ModalNavigationDrawer(
        drawerState = drawerState,
        drawerContent = {
            AppDrawer(
                currentRoute = currentScreen,
                navigate = navigate,
                logout = viewModel::logout,
                connect = connect,
                closeDrawer = { scope.launch { drawerState.close() } },
                connectionViewModel = viewModel
            )
        }
    ) {
        Scaffold(
            topBar = {
                TopAppBar(
                    title = { Text(openObject?.name ?: getScreenTitle(currentScreen), maxLines = 1) },
                    navigationIcon = {
                        if (openObject != null) {
                            IconButton(onClick = { openObis = null }) {
                                Icon(Icons.AutoMirrored.Filled.ArrowBack, "Back")
                            }
                        } else if (parentRoute != null) {
                            IconButton(onClick = { navigate(parentRoute) }) {
                                Icon(Icons.AutoMirrored.Filled.ArrowBack, "Back")
                            }
                        } else {
                            IconButton(onClick = { scope.launch { drawerState.open() } }) {
                                Icon(Icons.Default.Menu, "Menu")
                            }
                        }
                    }
                )
            }
        ) { padding ->
            val browsedClass = BROWSER_CLASSES[currentScreen]
            when {
                showScanner -> BleScannerScreen(
                    onDeviceSelected = { device, name ->
                        viewModel.connectToDevice(device, name)
                        showScanner = false
                    },
                    onClose = { showScanner = false }
                )
                browsedClass != null && openObject != null -> ObjectDetailScreen(
                    obj = openObject,
                    connectionViewModel = viewModel,
                    browserViewModel = browserViewModel,
                    onConnect = connect,
                    modifier = Modifier.padding(padding)
                )
                browsedClass != null -> ObjectListScreen(
                    classId = browsedClass,
                    connectionViewModel = viewModel,
                    browserViewModel = browserViewModel,
                    onOpen = { openObis = it.obis.toString() },
                    onConnect = connect,
                    modifier = Modifier.padding(padding)
                )
                currentScreen == "keys" -> SecurityKeysScreen(
                    viewModel = viewModel,
                    onDone = { navigate(if (keysFromSettings) "settings" else "home") },
                    modifier = Modifier.padding(padding)
                )
                currentScreen == "settings" -> SettingsScreen(
                    connectionViewModel = viewModel,
                    browserViewModel = browserViewModel,
                    onOpenKeys = { keysFromSettings = true; navigate("keys") },
                    onOpenSavedMeters = { navigate("saved") },
                    modifier = Modifier.padding(padding)
                )
                currentScreen == "saved" -> SavedMetersScreen(
                    browserViewModel = browserViewModel,
                    modifier = Modifier.padding(padding)
                )
                else -> HomeContent(
                    viewModel = viewModel,
                    onNavigate = navigate,
                    onScan = { showScanner = true },
                    modifier = Modifier.padding(padding)
                )
            }
        }
    }
}

/** Drawer routes that browse one interface class. */
private val BROWSER_CLASSES = mapOf(
    "data" to InterfaceClass.DATA,
    "register" to InterfaceClass.REGISTER,
    "extregister" to InterfaceClass.EXTENDED_REGISTER,
    "profile" to InterfaceClass.PROFILE_GENERIC,
    "clock" to InterfaceClass.CLOCK,
)

/** Pages reached from Settings, and where Back takes them. */
private val SUB_PAGES = mapOf("saved" to "settings", "keys" to "settings")

private fun getScreenTitle(screen: String): String = when (screen) {
    "home" -> "Meter Link"
    "data" -> "Data"
    "register" -> "Register"
    "extregister" -> "Extended Register"
    "profile" -> "Profile Generic"
    "clock" -> "Clock"
    "settings" -> "Settings"
    "saved" -> "Saved meters"
    "keys" -> "Security Keys"
    else -> "Meter Link"
}

@SuppressLint("MissingPermission")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeContent(
    modifier: Modifier = Modifier,
    viewModel: MeterConnectionViewModel,
    onNavigate: (String) -> Unit = {},
    onScan: () -> Unit = {},
) {
    val uiState by viewModel.uiState.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    val scope = rememberCoroutineScope()

    run {
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
                      Box(modifier = Modifier.fillMaxWidth()) {
                        if (uiState.connectionState is BleConnectionState.Connected) {
                            TextButton(
                                onClick = { viewModel.disconnect() },
                                modifier = Modifier
                                    .align(Alignment.TopEnd)
                                    .padding(top = 8.dp, end = 8.dp),
                                colors = ButtonDefaults.textButtonColors(
                                    contentColor = MaterialTheme.colorScheme.error
                                ),
                                contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp)
                            ) {
                                Icon(
                                    imageVector = Icons.Default.Close,
                                    contentDescription = null,
                                    modifier = Modifier.size(14.dp)
                                )
                                Spacer(Modifier.width(6.dp))
                                Text(
                                    text = "Disconnect",
                                    style = MaterialTheme.typography.labelSmall
                                )
                            }
                        }
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
                                    text = uiState.selectedDeviceName ?: "Unknown Device",
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
                                        Text("Reconnect to ${uiState.selectedDeviceName ?: "Device"}")
                                    }
                                }

                                Button(
                                    onClick = onScan,
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
                                text = "Browse the meter",
                                style = MaterialTheme.typography.titleMedium,
                                fontWeight = FontWeight.SemiBold
                            )

                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                horizontalArrangement = Arrangement.spacedBy(12.dp)
                            ) {
                                QuickActionCard(
                                    title = "Registers",
                                    icon = Icons.Default.Speed,
                                    onClick = { onNavigate("register") },
                                    modifier = Modifier.weight(1f)
                                )
                                QuickActionCard(
                                    title = "Profiles",
                                    icon = Icons.Default.TableChart,
                                    onClick = { onNavigate("profile") },
                                    modifier = Modifier.weight(1f)
                                )
                                QuickActionCard(
                                    title = "Clock",
                                    icon = Icons.Default.Schedule,
                                    onClick = { onNavigate("clock") },
                                    modifier = Modifier.weight(1f)
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