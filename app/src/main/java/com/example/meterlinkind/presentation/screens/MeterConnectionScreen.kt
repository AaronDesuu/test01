package com.example.meterlinkind.presentation.screens

import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material3.Button
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.foundation.clickable
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.window.Dialog
import com.example.meterlinkind.data.model.NewMeterListMode
import androidx.compose.material3.TextButton
import com.example.meterlinkind.data.repository.SavedMeter
import java.text.DateFormat
import java.util.Date
import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.app.Activity
import android.widget.Toast
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.animation.scaleIn
import androidx.compose.animation.scaleOut
import androidx.compose.material.icons.filled.PowerSettingsNew
import androidx.compose.material3.OutlinedIconButton
import androidx.compose.material3.IconButtonDefaults
import com.example.meterlinkind.presentation.components.OnlineColor
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.activity.compose.BackHandler
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Alignment
import androidx.compose.animation.slideInVertically
import androidx.compose.animation.core.tween
import com.example.meterlinkind.presentation.components.MeterCardMorph
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.FastOutSlowInEasing
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.animation.core.MutableTransitionState
import com.example.meterlinkind.presentation.components.rememberFailureShake
import com.example.meterlinkind.presentation.components.MeterStatusTitle
import com.example.meterlinkind.presentation.components.MeterStatusBadge
import com.example.meterlinkind.presentation.components.MeterStatusPill
import kotlin.math.roundToInt
import androidx.compose.ui.unit.IntOffset
import androidx.compose.animation.shrinkVertically
import androidx.compose.animation.fadeOut
import androidx.compose.animation.fadeIn
import androidx.compose.animation.expandVertically
import androidx.compose.animation.AnimatedVisibility
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
import com.example.meterlinkind.presentation.components.OBJECT_ENTRIES
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
        // After login, open the keys first so they can be checked before connecting.
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

    // Back handlers: the one declared last wins, so they go from least to most specific.
    // Home: a second Back within 2 s leaves the app.
    val context = LocalContext.current
    var lastBackOnHome by remember { mutableLongStateOf(0L) }
    BackHandler(enabled = currentScreen == "home") {
        val now = System.currentTimeMillis()
        if (now - lastBackOnHome < EXIT_CONFIRM_MS) {
            (context as? Activity)?.finish()
        } else {
            lastBackOnHome = now
            Toast.makeText(context, "Press back again to exit", Toast.LENGTH_SHORT).show()
        }
    }
    // Any other screen goes back to Home.
    BackHandler(enabled = currentScreen != "home") { navigate("home") }

    BackHandler(enabled = openObject != null) { openObis = null }

    // Sub-pages of Settings go back to Settings; Security Keys right after login goes on to Home.
    val parentRoute = SUB_PAGES[currentScreen]?.takeIf { currentScreen != "keys" || keysFromSettings }
    BackHandler(enabled = openObject == null && parentRoute != null) { navigate(parentRoute!!) }

    var showScanner by rememberSaveable { mutableStateOf(false) }
    // Declared last, so Back closes the scanner before anything else.
    BackHandler(enabled = showScanner) { showScanner = false }
    BackHandler(enabled = drawerState.isOpen) { scope.launch { drawerState.slideClosed() } }
    // From any screen: reconnect to the last meter, or scan for one.
    val connect = {
        val device = connection.selectedDevice
        if (device != null) {
            viewModel.connectToDevice(device)
        } else {
            navigate("home")
            showScanner = true
        }
    }

    // The object list belongs to a meter (serial) and client; load the stored one when either changes.
    LaunchedEffect(connection.serialNumber, loggedInClient) {
        browserViewModel.onMeter(connection.serialNumber, loggedInClient)
    }
    val browserState by browserViewModel.uiState.collectAsState()
    val reuseSource = browserState.reuseCandidate
    if (browserState.askReuse && reuseSource != null) {
        ReuseListDialog(
            meter = browserState.meterSerial ?: "This meter",
            source = reuseSource,
            onReuse = { remember ->
                if (remember) browserViewModel.setNewMeterListMode(NewMeterListMode.REUSE)
                browserViewModel.reuseList()
            },
            onRead = { remember ->
                if (remember) browserViewModel.setNewMeterListMode(NewMeterListMode.READ)
                browserViewModel.dismissReuse(read = true)
            },
            onDismiss = { browserViewModel.dismissReuse(read = false) },
        )
    }

    ModalNavigationDrawer(
        drawerState = drawerState,
        drawerContent = {
            AppDrawer(
                currentRoute = currentScreen,
                navigate = navigate,
                logout = viewModel::logout,
                connect = connect,
                closeDrawer = { scope.launch { drawerState.slideClosed() } },
                connectionViewModel = viewModel
            )
        }
    ) {
        Scaffold(
            topBar = {
                CenterAlignedTopAppBar(
                    title = {
                        Text(openObject?.name ?: getScreenTitle(currentScreen), maxLines = 1)
                    },
                    actions = {
                        // Object screens show the meter's status as a pill.
                        if (currentScreen in BROWSER_CLASSES) {
                            val online = connection.connectionState is BleConnectionState.Connected
                            MeterStatusPill(
                                deviceName = connection.selectedDeviceName,
                                connectionState = connection.connectionState,
                                associated = connection.isDlmsConnected,
                                onClick = {
                                    val connecting = connection.connectionState is BleConnectionState.Connecting
                                    if (online || connecting) navigate("home") else connect()
                                }
                            )
                        }
                    },
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
                browsedClass != null && openObject != null -> ObjectDetailScreen(
                    obj = openObject,
                    connectionViewModel = viewModel,
                    browserViewModel = browserViewModel,
                    modifier = Modifier.padding(padding)
                )
                browsedClass != null -> ObjectListScreen(
                    classId = browsedClass,
                    connectionViewModel = viewModel,
                    browserViewModel = browserViewModel,
                    onOpen = { openObis = it.obis.toString() },
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
                    showScanner = showScanner,
                    onScan = { showScanner = true },
                    onCloseScanner = { showScanner = false },
                    onDeviceSelected = { device, name ->
                        viewModel.connectToDevice(device, name)
                        showScanner = false
                    },
                    modifier = Modifier.padding(padding)
                )
            }
        }
    }
}

/**
 * Asks, for a meter new to the phone, whether to copy another meter's object
 * list or read its own. "Don't ask again" turns the choice made into the New
 * meters setting. Closing the dialog leaves the choice to the object list screens.
 */
@Composable
private fun ReuseListDialog(
    meter: String,
    source: SavedMeter,
    onReuse: (remember: Boolean) -> Unit,
    onRead: (remember: Boolean) -> Unit,
    onDismiss: () -> Unit,
) {
    var dontAsk by rememberSaveable { mutableStateOf(false) }
    val date = DateFormat.getDateInstance(DateFormat.MEDIUM).format(Date(source.readAtMillis))
    Dialog(onDismissRequest = onDismiss) {
        Surface(shape = RoundedCornerShape(28.dp), color = MaterialTheme.colorScheme.surfaceContainerHigh) {
            Column(
                modifier = Modifier.padding(horizontal = 24.dp, vertical = 24.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Text("New meter", style = MaterialTheme.typography.headlineSmall)
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    SerialChip(source.serial, MaterialTheme.colorScheme.surface, MaterialTheme.colorScheme.onSurface)
                    Icon(
                        Icons.AutoMirrored.Filled.ArrowForward,
                        contentDescription = "to",
                        modifier = Modifier.size(18.dp),
                        tint = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    SerialChip(meter, MaterialTheme.colorScheme.primaryContainer, MaterialTheme.colorScheme.onPrimaryContainer)
                }
                Text(
                    "Copy the saved object list to this meter?",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    textAlign = androidx.compose.ui.text.style.TextAlign.Center
                )
                Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                    Button(onClick = { onReuse(dontAsk) }, modifier = Modifier.fillMaxWidth()) {
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            Text("Use saved list")
                            Text(
                                "$date · ${source.objectCount} objects",
                                style = MaterialTheme.typography.labelSmall,
                                fontWeight = FontWeight.Normal
                            )
                        }
                    }
                    OutlinedButton(onClick = { onRead(dontAsk) }, modifier = Modifier.fillMaxWidth()) {
                        Text("Read from meter")
                    }
                }
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable { dontAsk = !dontAsk },
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        Text("Don't ask again", style = MaterialTheme.typography.bodyMedium)
                        Text(
                            "Change it in Settings",
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                    Switch(checked = dontAsk, onCheckedChange = { dontAsk = it })
                }
            }
        }
    }
}

/** A meter serial in a small rounded box. */
@Composable
private fun SerialChip(serial: String, background: Color, content: Color) {
    Text(
        serial,
        style = MaterialTheme.typography.labelMedium,
        fontFamily = FontFamily.Monospace,
        color = content,
        maxLines = 1,
        modifier = Modifier
            .background(background, RoundedCornerShape(8.dp))
            .padding(horizontal = 8.dp, vertical = 5.dp)
    )
}

/** Drawer routes that browse one interface class. */
private val BROWSER_CLASSES = mapOf(
    "data" to InterfaceClass.DATA,
    "register" to InterfaceClass.REGISTER,
    "extregister" to InterfaceClass.EXTENDED_REGISTER,
    "profile" to InterfaceClass.PROFILE_GENERIC,
    "disconnect" to InterfaceClass.DISCONNECT_CONTROL,
    "limiter" to InterfaceClass.LIMITER,
    "clock" to InterfaceClass.CLOCK,
)

/** Window for the second Back on Home that leaves the app. */
private const val EXIT_CONFIRM_MS = 2000L

/** Pages reached from Settings, and where Back takes them. */
private val SUB_PAGES = mapOf("saved" to "settings", "keys" to "settings")

private fun getScreenTitle(screen: String): String = when (screen) {
    "home" -> "Meter Link"
    "data" -> "Data"
    "register" -> "Register"
    "extregister" -> "Extended Register"
    "profile" -> "Profile Generic"
    "disconnect" -> "Disconnect Control"
    "limiter" -> "Limiter"
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
    showScanner: Boolean = false,
    onScan: () -> Unit = {},
    onCloseScanner: () -> Unit = {},
    onDeviceSelected: (BluetoothDevice, String?) -> Unit = { _, _ -> },
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
                contentAlignment = Alignment.TopCenter
            ) {
                // The card sits in the middle while idle with the scanner closed, at the top
                // otherwise. The gap above it is computed from the measured sizes.
                var areaHeight by remember { mutableIntStateOf(0) }
                var cardHeight by remember { mutableIntStateOf(0) }
                var buttonsHeight by remember { mutableIntStateOf(0) }
                val centered = uiState.connectionState.let {
                    it is BleConnectionState.Disconnected || it is BleConnectionState.Error
                } && !showScanner
                val density = LocalDensity.current
                val measured = areaHeight > 0 && cardHeight > 0 && (!centered || buttonsHeight > 0)
                val targetGap = if (centered && measured) {
                    with(density) {
                        val free = areaHeight - 16.dp.toPx() - cardHeight - 16.dp.toPx() - buttonsHeight
                        // Slightly above the true middle.
                        (free / 2f * 0.85f).coerceAtLeast(0f)
                    }
                } else 0f
                val topGap = remember { Animatable(0f) }
                var placed by remember { mutableStateOf(false) }
                LaunchedEffect(targetGap, measured) {
                    if (!placed) {
                        // First layout: no slide.
                        topGap.snapTo(targetGap)
                        if (measured) placed = true
                    } else {
                        topGap.animateTo(targetGap, tween(450, easing = FastOutSlowInEasing))
                    }
                }
                Column(
                    modifier = Modifier
                        .widthIn(max = 500.dp)
                        .fillMaxHeight()
                        .onSizeChanged { areaHeight = it.height }
                        .padding(start = 16.dp, top = 16.dp, end = 16.dp),
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    // Meter card; the area below it scrolls.
                    val state = uiState.connectionState
                    val failed = state is BleConnectionState.Error
                    val shake = rememberFailureShake(failed)
                    val copied: (String) -> Unit = { text ->
                        scope.launch {
                            snackbarHostState.showSnackbar("Copied: $text", duration = SnackbarDuration.Short)
                        }
                    }
                    ElevatedCard(
                        modifier = Modifier
                            .padding(top = with(density) { topGap.value.toDp() })
                            .fillMaxWidth()
                            .onSizeChanged { cardHeight = it.height }
                            .offset { IntOffset(shake.value.roundToInt(), 0) },
                        elevation = CardDefaults.elevatedCardElevation(defaultElevation = 8.dp)
                    ) {
                        // Big card while idle, compact row otherwise.
                        val compactness by animateFloatAsState(
                            if (centered) 0f else 1f,
                            tween(450, easing = FastOutSlowInEasing),
                            label = "card compactness"
                        )
                        MeterCardMorph(
                            progress = compactness,
                            badge = { size -> MeterStatusBadge(state, size = size) },
                            big = {
                                Column(Modifier.fillMaxWidth(), horizontalAlignment = Alignment.CenterHorizontally) {
                                    MeterStatusTitle(
                                        when (state) {
                                            is BleConnectionState.Connected -> "Connected"
                                            is BleConnectionState.Connecting -> "Connecting..."
                                            is BleConnectionState.Error -> "Connection Failed"
                                            else -> "Not Connected"
                                        }
                                    )
                                    var bigFailure by remember { mutableStateOf("") }
                                    (state as? BleConnectionState.Error)?.let { bigFailure = it.message }
                                    if (failed) {
                                        Text(
                                            text = bigFailure,
                                            style = MaterialTheme.typography.bodyMedium,
                                            color = MaterialTheme.colorScheme.error,
                                            textAlign = androidx.compose.ui.text.style.TextAlign.Center,
                                            modifier = Modifier.padding(top = 4.dp)
                                        )
                                    }
                                    val device = uiState.selectedDevice
                                    if (device != null) {
                                        Spacer(Modifier.height(8.dp))
                                        CopyableText(
                                            text = uiState.selectedDeviceName ?: "Unknown Device",
                                            style = MaterialTheme.typography.titleMedium,
                                            onCopied = copied
                                        )
                                        CopyableText(text = device.address, onCopied = copied)
                                    } else {
                                        Text(
                                            text = "Scan to find a meter nearby",
                                            style = MaterialTheme.typography.bodyMedium,
                                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                                            modifier = Modifier.padding(top = 4.dp)
                                        )
                                    }
                                }
                            },
                            compact = {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            verticalAlignment = Alignment.CenterVertically,
                            horizontalArrangement = Arrangement.spacedBy(14.dp)
                        ) {
                            Column(modifier = Modifier.weight(1f)) {
                                MeterStatusTitle(
                                    text = when (state) {
                                        is BleConnectionState.Connected -> "CONNECTED"
                                        is BleConnectionState.Connecting -> "CONNECTING..."
                                        is BleConnectionState.Error -> "CONNECTION FAILED"
                                        else -> "NOT CONNECTED"
                                    },
                                    style = MaterialTheme.typography.labelMedium,
                                    color = when (state) {
                                        is BleConnectionState.Connected -> OnlineColor
                                        is BleConnectionState.Connecting -> MaterialTheme.colorScheme.primary
                                        is BleConnectionState.Error -> MaterialTheme.colorScheme.error
                                        else -> MaterialTheme.colorScheme.onSurfaceVariant
                                    }
                                )
                                if (uiState.selectedDevice != null) {
                                    CopyableText(
                                        text = uiState.selectedDeviceName ?: "Unknown Device",
                                        style = MaterialTheme.typography.titleLarge,
                                        color = MaterialTheme.colorScheme.onSurface,
                                        fontWeight = FontWeight.Bold,
                                        onCopied = copied
                                    )
                                } else {
                                    Text(
                                        text = "No meter",
                                        style = MaterialTheme.typography.titleLarge,
                                        fontWeight = FontWeight.Bold
                                    )
                                }

                                // Kept for the fold-away animation after the error clears.
                                var lastFailure by remember { mutableStateOf("") }
                                (state as? BleConnectionState.Error)?.let { lastFailure = it.message }
                                AnimatedVisibility(
                                    visible = failed,
                                    enter = expandVertically() + fadeIn(),
                                    exit = shrinkVertically() + fadeOut()
                                ) {
                                    Text(
                                        text = lastFailure,
                                        style = MaterialTheme.typography.bodySmall,
                                        color = MaterialTheme.colorScheme.error,
                                        modifier = Modifier.padding(top = 2.dp)
                                    )
                                }

                                val device = uiState.selectedDevice
                                if (device == null) {
                                    Text(
                                        text = "Scan to find a meter nearby",
                                        style = MaterialTheme.typography.bodySmall,
                                        color = MaterialTheme.colorScheme.onSurfaceVariant
                                    )
                                }
                                AnimatedVisibility(
                                    visible = device != null && !failed,
                                    enter = expandVertically() + fadeIn(),
                                    exit = shrinkVertically() + fadeOut()
                                ) {
                                    var lastAddress by remember { mutableStateOf("") }
                                    device?.let { lastAddress = it.address }
                                    val serial = uiState.serialNumber?.takeIf {
                                        it.isNotEmpty() && state is BleConnectionState.Connected
                                    }
                                    Column {
                                        if (serial != null) Row(verticalAlignment = Alignment.CenterVertically) {
                                            Text(
                                                text = "S/N: ",
                                                style = MaterialTheme.typography.bodySmall,
                                                color = MaterialTheme.colorScheme.onSurfaceVariant
                                            )
                                            CopyableText(
                                                text = serial,
                                                style = MaterialTheme.typography.bodySmall,
                                                fontWeight = FontWeight.SemiBold,
                                                onCopied = copied
                                            )
                                            if (serial == "0000000000") {
                                                IconButton(
                                                    onClick = { viewModel.fetchSerialNumber() },
                                                    modifier = Modifier.size(20.dp)
                                                ) {
                                                    Icon(
                                                        Icons.Default.Refresh,
                                                        contentDescription = "Refresh Serial Number",
                                                        modifier = Modifier.size(14.dp),
                                                        tint = MaterialTheme.colorScheme.primary
                                                    )
                                                }
                                            }
                                        }
                                        CopyableText(
                                            text = lastAddress,
                                            style = MaterialTheme.typography.bodySmall,
                                            onCopied = copied
                                        )
                                    }
                                }
                            }

                            AnimatedVisibility(
                                visible = state is BleConnectionState.Connected,
                                enter = scaleIn() + fadeIn(),
                                exit = scaleOut() + fadeOut()
                            ) {
                                OutlinedIconButton(
                                    onClick = { viewModel.disconnect() },
                                    colors = IconButtonDefaults.outlinedIconButtonColors(
                                        contentColor = MaterialTheme.colorScheme.error
                                    )
                                ) {
                                    Icon(Icons.Default.PowerSettingsNew, contentDescription = "Disconnect")
                                }
                            }
                        }
                            }
                        )
                    }

                    // Scrolls under the meter card.
                    Column(
                        modifier = Modifier
                            .weight(1f)
                            .fillMaxWidth()
                            .verticalScroll(rememberScrollState())
                            .padding(bottom = 16.dp),
                        verticalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                    when (uiState.connectionState) {
                        is BleConnectionState.Disconnected, is BleConnectionState.Error -> {
                            Column(
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .onSizeChanged { if (!showScanner) buttonsHeight = it.height },
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

                                if (showScanner) {
                                    BleScannerPanel(onDeviceSelected = onDeviceSelected, onClose = onCloseScanner)
                                } else {
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
                        }

                        is BleConnectionState.Connected -> {
                            // Rows slide down one after another, out from under the card.
                            SlideDownIn(index = 0) {
                                Text(
                                    text = "Browse the meter",
                                    style = MaterialTheme.typography.titleMedium,
                                    fontWeight = FontWeight.SemiBold
                                )
                            }

                            // Spacers keep the tiles of a short last row the same width.
                            OBJECT_ENTRIES.chunked(3).forEachIndexed { row, pair ->
                                SlideDownIn(index = row + 1) {
                                    Row(
                                        modifier = Modifier.fillMaxWidth(),
                                        horizontalArrangement = Arrangement.spacedBy(12.dp)
                                    ) {
                                        pair.forEach { entry ->
                                            QuickActionCard(
                                                title = entry.label,
                                                icon = entry.icon,
                                                onClick = { onNavigate(entry.route) },
                                                modifier = Modifier.weight(1f)
                                            )
                                        }
                                        repeat(3 - pair.size) { Spacer(Modifier.weight(1f)) }
                                    }
                                }
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
            }

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

/** Slides [content] down and fades it in when first shown, [index] × 60 ms after the first row. */
@Composable
private fun SlideDownIn(index: Int, content: @Composable () -> Unit) {
    val visible = remember { MutableTransitionState(false).apply { targetState = true } }
    AnimatedVisibility(
        visibleState = visible,
        enter = slideInVertically(tween(380, delayMillis = index * 60)) { -it } +
            fadeIn(tween(380, delayMillis = index * 60)),
    ) { content() }
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
        modifier = modifier.height(104.dp),
        colors = CardDefaults.elevatedCardColors(
            containerColor = containerColor
        )
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 6.dp, vertical = 12.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Icon(
                imageVector = icon,
                contentDescription = null,
                modifier = Modifier.size(28.dp),
                tint = contentColor
            )
            Spacer(Modifier.height(6.dp))
            Text(
                text = title,
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Medium,
                color = contentColor,
                textAlign = androidx.compose.ui.text.style.TextAlign.Center,
                maxLines = 2
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
/** Closes the drawer over 300 ms; Material 3's close() is so fast the drawer seems to vanish. */
private suspend fun DrawerState.slideClosed() =
    animateTo(DrawerValue.Closed, tween(DRAWER_CLOSE_MS, easing = FastOutSlowInEasing))

private const val DRAWER_CLOSE_MS = 300
