package com.example.meterlinkind.presentation.screens

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.SignalCellularAlt
import androidx.compose.material.icons.filled.SignalCellularAlt1Bar
import androidx.compose.material.icons.filled.SignalCellularAlt2Bar
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.PrimaryTabRow
import androidx.compose.material3.Surface
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat
import com.example.meterlinkind.data.repository.BleRepository

/**
 * Finds meters nearby, shown as a panel under the Meter screen's meter card.
 * Scanning starts when it appears and stops when it's closed or a device is picked.
 */
@SuppressLint("MissingPermission")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BleScannerPanel(
    onDeviceSelected: (BluetoothDevice, String?) -> Unit,
    onClose: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val context = LocalContext.current
    var isScanning by remember { mutableStateOf(false) }
    var eMeterDevices by remember { mutableStateOf<List<BluetoothDevice>>(emptyList()) }
    var unknownDevices by remember { mutableStateOf<List<BluetoothDevice>>(emptyList()) }
    // Names by address, from the advertisements. BluetoothDevice.name is Android's
    // cache, which drops unbonded devices once a GATT connection to them closes.
    var deviceNames by remember { mutableStateOf<Map<String, String>>(emptyMap()) }
    // Latest signal strength (dBm) per address, from each advertisement.
    var rssiByAddress by remember { mutableStateOf<Map<String, Int>>(emptyMap()) }
    var selectedTabIndex by remember { mutableIntStateOf(0) }
    var hasPermissions by remember { mutableStateOf(checkPermissions(context)) }

    val bluetoothManager = context.getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager
    val bluetoothAdapter = bluetoothManager?.adapter
    var isBluetoothEnabled by remember { mutableStateOf(bluetoothAdapter?.isEnabled == true) }
    val canScan by remember(hasPermissions, isBluetoothEnabled) {
        derivedStateOf { hasPermissions && isBluetoothEnabled && bluetoothAdapter?.bluetoothLeScanner != null }
    }

    val permissionLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.RequestMultiplePermissions()
    ) { _ ->
        hasPermissions = checkPermissions(context)
    }

    val enableBluetoothLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.StartActivityForResult()
    ) { _ ->
        isBluetoothEnabled = bluetoothAdapter?.isEnabled == true
    }

    val scanCallback = remember {
        object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                val device = result.device
                val deviceName = result.scanRecord?.deviceName ?: device.name
                if (deviceName != null && deviceNames[device.address] != deviceName) {
                    deviceNames = deviceNames + (device.address to deviceName)
                }
                if (rssiByAddress[device.address] != result.rssi) {
                    rssiByAddress = rssiByAddress + (device.address to result.rssi)
                }
                val advertisesMeterService = result.scanRecord?.serviceUuids
                    ?.any { it.uuid == BleRepository.ADVERTISED_SERVICE_UUID } == true

                // F5LWF meters advertise the same UUID, so the UUID only counts
                // when the name is missing from this advertisement.
                if (deviceName?.contains("FT5AB") == true || (deviceName == null && advertisesMeterService)) {
                    if (!eMeterDevices.contains(device)) {
                        eMeterDevices = eMeterDevices + device
                    }
                } else {
                    // All other devices go to Unknown
                    if (!unknownDevices.contains(device)) {
                        unknownDevices = unknownDevices + device
                    }
                }
            }
        }
    }

    // Request permissions on first composition if missing.
    LaunchedEffect(Unit) {
        if (!hasPermissions) {
            permissionLauncher.launch(getRequiredPermissions())
        }
    }

    // Auto-start scan once we have permissions and Bluetooth is on.
    LaunchedEffect(canScan) {
        if (canScan) {
            eMeterDevices = emptyList()
            unknownDevices = emptyList()
            isScanning = true
            bluetoothAdapter?.bluetoothLeScanner?.startScan(scanCallback)
        }
    }

    DisposableEffect(Unit) {
        onDispose {
            if (isScanning) {
                isScanning = false
                bluetoothAdapter?.bluetoothLeScanner?.stopScan(scanCallback)
            }
        }
    }

    Surface(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(16.dp),
        color = MaterialTheme.colorScheme.surfaceContainerLow,
    ) {
        Column(modifier = Modifier.padding(vertical = 8.dp)) {
            when {
                bluetoothAdapter == null -> {
                    BlockingMessage(
                        message = "This device does not support Bluetooth."
                    )
                }
                !hasPermissions -> {
                    BlockingMessage(
                        message = "Bluetooth permissions are required to scan for meters.",
                        actionLabel = "Grant permissions",
                        onAction = { permissionLauncher.launch(getRequiredPermissions()) }
                    )
                }
                !isBluetoothEnabled -> {
                    BlockingMessage(
                        message = "Bluetooth is turned off.",
                        actionLabel = "Turn on Bluetooth",
                        onAction = {
                            enableBluetoothLauncher.launch(Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE))
                        }
                    )
                }
                else -> {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(start = 8.dp, end = 4.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        PrimaryTabRow(
                            selectedTabIndex = selectedTabIndex,
                            containerColor = MaterialTheme.colorScheme.surfaceContainerLow,
                            modifier = Modifier.weight(1f)
                        ) {
                            Tab(
                                selected = selectedTabIndex == 0,
                                onClick = { selectedTabIndex = 0 },
                                text = { Text("E-Meters (${eMeterDevices.size})") }
                            )
                            Tab(
                                selected = selectedTabIndex == 1,
                                onClick = { selectedTabIndex = 1 },
                                text = { Text("Unknown (${unknownDevices.size})") }
                            )
                        }

                        IconButton(
                            onClick = {
                                eMeterDevices = emptyList()
                                unknownDevices = emptyList()
                                isScanning = true
                                bluetoothAdapter.bluetoothLeScanner?.startScan(scanCallback)
                            }
                        ) {
                            Icon(
                                imageVector = Icons.Default.Refresh,
                                contentDescription = "Scan again"
                            )
                        }

                        IconButton(
                            onClick = {
                                if (isScanning) {
                                    isScanning = false
                                    bluetoothAdapter.bluetoothLeScanner?.stopScan(scanCallback)
                                }
                                onClose()
                            }
                        ) {
                            Icon(
                                imageVector = Icons.Default.Close,
                                contentDescription = "Close scanner"
                            )
                        }
                    }

                    when (selectedTabIndex) {
                        0 -> DeviceList(eMeterDevices, deviceNames, rssiByAddress, onDeviceSelected)
                        1 -> DeviceList(unknownDevices, deviceNames, rssiByAddress, onDeviceSelected)
                    }
                }
            }
        }
    }
}

@Composable
private fun BlockingMessage(
    message: String,
    actionLabel: String? = null,
    onAction: (() -> Unit)? = null,
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(24.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        Text(
            text = message,
            style = MaterialTheme.typography.bodyLarge,
        )
        if (actionLabel != null && onAction != null) {
            Spacer(modifier = Modifier.height(16.dp))
            Button(onClick = onAction) { Text(actionLabel) }
        }
    }
}

@SuppressLint("MissingPermission")
@Composable
private fun DeviceList(
    devices: List<BluetoothDevice>,
    names: Map<String, String>,
    rssi: Map<String, Int>,
    onDeviceSelected: (BluetoothDevice, String?) -> Unit
) {
    if (devices.isEmpty()) {
        Text(
            text = "No devices found",
            modifier = Modifier.padding(16.dp),
            style = MaterialTheme.typography.bodyMedium
        )
    } else {
        // Not a LazyColumn: the panel sits inside the Meter screen's scrolling column.
        Column {
            devices.forEach { device ->
                Card(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 16.dp, vertical = 8.dp)
                        .clickable { onDeviceSelected(device, names[device.address]) }
                ) {
                    Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
                        Column(modifier = Modifier.weight(1f)) {
                            Text(
                                text = names[device.address] ?: "Unknown Device",
                                style = MaterialTheme.typography.titleMedium
                            )
                            Text(
                                text = device.address,
                                style = MaterialTheme.typography.bodySmall
                            )
                        }
                        rssi[device.address]?.let { SignalStrength(it) }
                    }
                }
            }
        }
    }
}

/** Bars and dBm for one device: 3 bars from -65 dBm, 2 from -80, else 1. */
@Composable
private fun SignalStrength(dbm: Int) {
    val icon = when {
        dbm >= -65 -> Icons.Default.SignalCellularAlt
        dbm >= -80 -> Icons.Default.SignalCellularAlt2Bar
        else -> Icons.Default.SignalCellularAlt1Bar
    }
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Icon(icon, contentDescription = "Signal $dbm dBm", tint = MaterialTheme.colorScheme.primary)
        Text("$dbm dBm", style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
    }
}

private fun getRequiredPermissions(): Array<String> {
    return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
        arrayOf(
            Manifest.permission.BLUETOOTH_SCAN,
            Manifest.permission.BLUETOOTH_CONNECT,
            Manifest.permission.ACCESS_FINE_LOCATION
        )
    } else {
        arrayOf(
            Manifest.permission.BLUETOOTH,
            Manifest.permission.BLUETOOTH_ADMIN,
            Manifest.permission.ACCESS_FINE_LOCATION
        )
    }
    
}

private fun checkPermissions(context: Context): Boolean {
    return getRequiredPermissions().all {
        ContextCompat.checkSelfPermission(context, it) == PackageManager.PERMISSION_GRANTED
    }
}