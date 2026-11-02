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
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.PrimaryTabRow
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
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

@SuppressLint("MissingPermission")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BleScannerScreen(
    onDeviceSelected: (BluetoothDevice) -> Unit,
    onClose: () -> Unit
) {
    val context = LocalContext.current
    var isScanning by remember { mutableStateOf(false) }
    var eMeterDevices by remember { mutableStateOf<List<BluetoothDevice>>(emptyList()) }
    var unknownDevices by remember { mutableStateOf<List<BluetoothDevice>>(emptyList()) }
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
                val deviceName = device.name ?: result.scanRecord?.deviceName
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

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Scan for Meters") },
                navigationIcon = {
                    IconButton(onClick = {
                        if (isScanning) {
                            isScanning = false
                            bluetoothAdapter?.bluetoothLeScanner?.stopScan(scanCallback)
                        }
                        onClose()
                    }) {
                        Text("←")
                    }
                }
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
        ) {
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
                            .padding(horizontal = 16.dp, vertical = 8.dp),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        PrimaryTabRow(
                            selectedTabIndex = selectedTabIndex,
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
                                contentDescription = "Refresh"
                            )
                        }

                        IconButton(
                            onClick = {
                                isScanning = false
                                bluetoothAdapter.bluetoothLeScanner?.stopScan(scanCallback)
                            },
                            enabled = isScanning
                        ) {
                            Icon(
                                imageVector = Icons.Default.Close,
                                contentDescription = "Stop Scan"
                            )
                        }
                    }

                    when (selectedTabIndex) {
                        0 -> DeviceList(devices = eMeterDevices, onDeviceSelected = onDeviceSelected)
                        1 -> DeviceList(devices = unknownDevices, onDeviceSelected = onDeviceSelected)
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
            .fillMaxSize()
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
    onDeviceSelected: (BluetoothDevice) -> Unit
) {
    if (devices.isEmpty()) {
        Text(
            text = "No devices found",
            modifier = Modifier.padding(16.dp),
            style = MaterialTheme.typography.bodyMedium
        )
    } else {
        LazyColumn {
            items(devices) { device ->
                Card(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 16.dp, vertical = 8.dp)
                        .clickable { onDeviceSelected(device) }
                ) {
                    Column(modifier = Modifier.padding(16.dp)) {
                        Text(
                            text = device.name ?: "Unknown Device",
                            style = MaterialTheme.typography.titleMedium
                        )
                        Text(
                            text = device.address,
                            style = MaterialTheme.typography.bodySmall
                        )
                    }
                }
            }
        }
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