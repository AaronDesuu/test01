package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Download
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.Schedule
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.PrimaryTabRow
import androidx.compose.material3.Surface
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.protocol.AccessMode
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.data.protocol.DisconnectControl
import com.example.meterlinkind.data.protocol.Limiter
import com.example.meterlinkind.data.protocol.CosemObject
import com.example.meterlinkind.data.protocol.CosemStructures
import com.example.meterlinkind.data.protocol.DlmsData
import com.example.meterlinkind.data.protocol.InterfaceClass
import com.example.meterlinkind.data.protocol.ObisCode
import com.example.meterlinkind.data.protocol.ScalerUnit
import com.example.meterlinkind.data.protocol.displayNumber
import com.example.meterlinkind.presentation.viewmodel.AttributeState
import com.example.meterlinkind.presentation.viewmodel.BufferSelection
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionUiState
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
import com.example.meterlinkind.presentation.viewmodel.ObjectBrowserUiState
import com.example.meterlinkind.presentation.viewmodel.ObjectBrowserViewModel
import com.example.meterlinkind.presentation.viewmodel.ProfileBufferState
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

/** One object's attribute table, plus the buffer reader (Profile Generic) or clock setting (Clock). */
@Composable
fun ObjectDetailScreen(
    obj: CosemObject,
    connectionViewModel: MeterConnectionViewModel,
    browserViewModel: ObjectBrowserViewModel,
    modifier: Modifier = Modifier,
) {
    val connection by connectionViewModel.uiState.collectAsState()
    val browser by browserViewModel.uiState.collectAsState()
    val online = connection.isDlmsConnected
    // Profile Generic gets two tabs: its attributes, and the buffer reader with its table.
    val hasBufferTab = obj.classId == InterfaceClass.PROFILE_GENERIC && obj.canRead(ObjectBrowserViewModel.BUFFER)
    var tab by rememberSaveable(obj.obis.toString()) { mutableIntStateOf(0) }

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(horizontal = 16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        if (hasBufferTab) {
            PrimaryTabRow(selectedTabIndex = tab) {
                Tab(selected = tab == 0, onClick = { tab = 0 }, text = { Text("Attributes") })
                Tab(selected = tab == 1, onClick = { tab = 1 }, text = { Text("Buffer") })
            }
        }
        val listModifier = Modifier
            .weight(1f)
            .fillMaxWidth()
        if (hasBufferTab && tab == 1) {
            LazyColumn(modifier = listModifier, verticalArrangement = Arrangement.spacedBy(8.dp)) {
                item {
                    BufferSection(
                        state = browser.buffers[obj.obis] ?: ProfileBufferState(),
                        scalers = browser.scalers,
                        decimals = browser.decimals,
                        defaultMode = browser.bufferReadMode,
                        enabled = online,
                        onRead = { selection -> browserViewModel.readBuffer(obj, selection) },
                    )
                }
                item { Text("") } // breathing room above the gesture bar
            }
        } else {
            AttributesTab(obj, connection, browser, browserViewModel, hasBufferTab, listModifier)
        }
    }
}

@Composable
private fun AttributesTab(
    obj: CosemObject,
    connection: MeterConnectionUiState,
    browser: ObjectBrowserUiState,
    browserViewModel: ObjectBrowserViewModel,
    hasBufferTab: Boolean,
    modifier: Modifier,
) {
    val online = connection.isDlmsConnected
    val (readable, unreadable) = (1..InterfaceClass.attributeCount(obj.classId)).partition { obj.canRead(it) }
    // Attributes this client can't read stay folded away under one row.
    var showUnreadable by rememberSaveable(obj.obis.toString()) { mutableStateOf(false) }
    val accessOf = { attribute: Int -> obj.attributeAccess?.let { AccessMode.label(it[attribute] ?: AccessMode.NONE) } }

    LazyColumn(modifier = modifier, verticalArrangement = Arrangement.spacedBy(8.dp)) {
        item {
            Row(
                modifier = Modifier.padding(vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column(modifier = Modifier.weight(1f)) {
                    Text(obj.obis.toString(), fontFamily = FontFamily.Monospace, style = MaterialTheme.typography.titleMedium)
                    Text(
                        "${InterfaceClass.name(obj.classId)} · class ${obj.classId} v${obj.version} · #${obj.index}",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                FilledTonalButton(onClick = { browserViewModel.readAll(obj) }, enabled = online) {
                    Icon(Icons.Default.Download, contentDescription = null, modifier = Modifier.size(18.dp))
                    Text("  Read all")
                }
            }
        }
        items(readable, key = { it }) { attribute ->
            if (hasBufferTab && attribute == ObjectBrowserViewModel.BUFFER) {
                AttributeRow(
                    attribute, InterfaceClass.attributeName(obj.classId, attribute), accessOf(attribute), null,
                    note = "Read it in the Buffer tab", enabled = false,
                ) {}
            } else {
                AttributeRow(
                    attribute = attribute,
                    name = InterfaceClass.attributeName(obj.classId, attribute),
                    access = accessOf(attribute),
                    state = browser.attribute(obj, attribute),
                    enabled = online,
                    format = { formatValue(obj.classId, attribute, it, browser.scalers[obj.obis], browser.decimals) },
                    onRead = { browserViewModel.read(obj, attribute) },
                )
            }
        }
        if (unreadable.isNotEmpty()) {
            item(key = "unreadable") {
                TextButton(onClick = { showUnreadable = !showUnreadable }) {
                    Icon(
                        if (showUnreadable) Icons.Default.ExpandLess else Icons.Default.ExpandMore,
                        contentDescription = null,
                        modifier = Modifier.size(18.dp)
                    )
                    Text("  ${unreadable.size} not readable by this client")
                }
            }
            if (showUnreadable) {
                items(unreadable, key = { it }) { attribute ->
                    AttributeRow(
                        attribute, InterfaceClass.attributeName(obj.classId, attribute), accessOf(attribute), null,
                        note = "Not readable by this client", enabled = false,
                    ) {}
                }
            }
        }
        if (obj.classId == InterfaceClass.DISCONNECT_CONTROL && connection.client == ClientProfile.UTILITY_SETTINGS) {
            item {
                RelayCard(
                    meter = connection.serialNumber ?: connection.selectedDeviceName ?: "the meter",
                    message = browser.relayMessage,
                    enabled = online,
                    onSwitch = { reconnect -> browserViewModel.switchRelay(obj, reconnect) },
                )
            }
        }
        if (obj.classId == InterfaceClass.CLOCK && connection.client == ClientProfile.UTILITY_SETTINGS &&
            obj.canWrite(ObjectBrowserViewModel.TIME)
        ) {
            item {
                SetClockCard(
                    meter = connection.serialNumber ?: connection.selectedDeviceName ?: "the meter",
                    clockName = obj.name,
                    message = browser.clockMessage,
                    enabled = online,
                    onSet = { browserViewModel.setClockToPhoneTime(obj) },
                )
            }
        }
        item { Text("") } // breathing room above the gesture bar
    }
}

@Composable
private fun AttributeRow(
    attribute: Int,
    name: String,
    /** "R", "R/W"... from the meter's object list; null if unknown. */
    access: String?,
    state: AttributeState?,
    note: String? = null,
    enabled: Boolean,
    format: (DlmsData) -> String = { it.display() },
    onRead: () -> Unit,
) {
    Surface(
        shape = RoundedCornerShape(12.dp),
        color = MaterialTheme.colorScheme.surfaceContainerLow,
        modifier = Modifier.fillMaxWidth()
    ) {
        Row(
            modifier = Modifier.padding(start = 12.dp, top = 8.dp, bottom = 8.dp, end = 4.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(2.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    Text(
                        "$attribute · $name",
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    access?.let { AccessTag(it) }
                }
                when {
                    state == null -> Text(
                        note.orEmpty(),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    state.error != null -> Text(state.error, style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.error)
                    state.value != null -> SelectionContainer {
                        Text(format(state.value), style = MaterialTheme.typography.bodyMedium, fontFamily = FontFamily.Monospace)
                    }
                    state.progress != null -> Text(state.progress, style = MaterialTheme.typography.bodySmall)
                    else -> Text("Not read", style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.onSurfaceVariant)
                }
            }
            if (state != null) {
                if (state.loading) {
                    CircularProgressIndicator(modifier = Modifier.padding(12.dp).size(20.dp), strokeWidth = 2.dp)
                } else {
                    IconButton(onClick = onRead, enabled = enabled) {
                        Icon(Icons.Default.Refresh, contentDescription = "Read $name")
                    }
                }
            }
        }
    }
}

/** The client's access to an attribute, as the meter granted it. */
@Composable
private fun AccessTag(label: String) {
    val none = label == "none"
    Text(
        label,
        style = MaterialTheme.typography.labelSmall,
        fontWeight = FontWeight.Medium,
        color = if (none) MaterialTheme.colorScheme.onSurfaceVariant else MaterialTheme.colorScheme.onSecondaryContainer,
        modifier = Modifier
            .background(
                if (none) MaterialTheme.colorScheme.surfaceVariant else MaterialTheme.colorScheme.secondaryContainer,
                RoundedCornerShape(6.dp)
            )
            .padding(horizontal = 6.dp, vertical = 1.dp)
    )
}

@Composable
private fun BufferSection(
    state: ProfileBufferState,
    scalers: Map<ObisCode, ScalerUnit>,
    decimals: Int?,
    defaultMode: BufferReadMode,
    enabled: Boolean,
    onRead: (BufferSelection) -> Unit,
) {
    Column(verticalArrangement = Arrangement.spacedBy(8.dp), modifier = Modifier.padding(top = 4.dp)) {
        state.entriesInUse?.let {
            Text("$it entries stored in the meter", style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }
        BufferReadingOptions(defaultMode = defaultMode, enabled = enabled, loading = state.loading, progress = state.progress, onRead = onRead)
        state.error?.let { Text(it, color = MaterialTheme.colorScheme.error, style = MaterialTheme.typography.bodyMedium) }
        state.rows?.let { rows -> BufferTable(state, rows, scalers, decimals) }
    }
}

@Composable
private fun BufferTable(
    state: ProfileBufferState,
    rows: List<List<DlmsData>>,
    scalers: Map<ObisCode, ScalerUnit>,
    decimals: Int?,
) {
    val width = rows.maxOfOrNull { it.size } ?: 0
    // Register columns get their unit in the header and scaled values in the cells.
    val columnScalers = List(width) { i ->
        state.columns?.getOrNull(i)?.takeIf { it.attribute == ObjectBrowserViewModel.VALUE }?.let { scalers[it.obis] }
    }
    val headers = List(width) { i ->
        val label = state.columns?.getOrNull(i)?.label ?: "Column ${i + 1}"
        columnScalers[i]?.symbol?.takeIf { it.isNotEmpty() }?.let { "$label ($it)" } ?: label
    }
    val cell = Modifier.width(150.dp).padding(horizontal = 8.dp, vertical = 6.dp)
    Text("${rows.size} rows", style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
    SelectionContainer {
        Column(
            modifier = Modifier
                .horizontalScroll(rememberScrollState())
                .background(MaterialTheme.colorScheme.surfaceContainerLow, RoundedCornerShape(12.dp))
        ) {
            Row(Modifier.background(MaterialTheme.colorScheme.secondaryContainer, RoundedCornerShape(topStart = 12.dp, topEnd = 12.dp))) {
                headers.forEach {
                    Text(it, modifier = cell, style = MaterialTheme.typography.labelMedium, color = MaterialTheme.colorScheme.onSecondaryContainer)
                }
            }
            rows.forEach { row ->
                Row {
                    row.forEachIndexed { i, value ->
                        Text(columnScalers.getOrNull(i)?.scale(value, decimals) ?: value.displayNumber(decimals), modifier = cell, style = MaterialTheme.typography.bodySmall, fontFamily = FontFamily.Monospace)
                    }
                }
            }
        }
    }
}

/** Gurux's Disconnect / Reconnect buttons; both switch the meter's relay, so each asks first. */
@Composable
private fun RelayCard(meter: String, message: String?, enabled: Boolean, onSwitch: (reconnect: Boolean) -> Unit) {
    var confirm by rememberSaveable { mutableStateOf<Boolean?>(null) } // true = reconnect
    Surface(
        shape = RoundedCornerShape(12.dp),
        color = MaterialTheme.colorScheme.errorContainer,
        contentColor = MaterialTheme.colorScheme.onErrorContainer,
        modifier = Modifier.fillMaxWidth().padding(top = 8.dp)
    ) {
        Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Text("Relay", style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.SemiBold)
            Text("Disconnect cuts the supply behind this meter; Reconnect restores it.", style = MaterialTheme.typography.bodySmall)
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Button(
                    onClick = { confirm = false },
                    enabled = enabled,
                    colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.error),
                    modifier = Modifier.weight(1f)
                ) { Text("Disconnect") }
                OutlinedButton(onClick = { confirm = true }, enabled = enabled, modifier = Modifier.weight(1f)) { Text("Reconnect") }
            }
            message?.let { Text(it, style = MaterialTheme.typography.bodyMedium) }
        }
    }
    confirm?.let { reconnect ->
        AlertDialog(
            onDismissRequest = { confirm = null },
            title = { Text(if (reconnect) "Reconnect $meter?" else "Disconnect $meter?") },
            text = {
                Text(
                    if (reconnect) "The meter's relay closes and supply is restored."
                    else "The meter's relay opens and the supply behind it is cut."
                )
            },
            confirmButton = {
                TextButton(onClick = { confirm = null; onSwitch(reconnect) }) { Text(if (reconnect) "Reconnect" else "Disconnect") }
            },
            dismissButton = { TextButton(onClick = { confirm = null }) { Text("Cancel") } },
        )
    }
}

@Composable
private fun SetClockCard(meter: String, clockName: String, message: String?, enabled: Boolean, onSet: () -> Unit) {
    var confirm by rememberSaveable { mutableStateOf(false) }
    Surface(
        shape = RoundedCornerShape(12.dp),
        color = MaterialTheme.colorScheme.tertiaryContainer,
        contentColor = MaterialTheme.colorScheme.onTertiaryContainer,
        modifier = Modifier.fillMaxWidth().padding(top = 8.dp)
    ) {
        Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Text("Set clock", style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.SemiBold)
            Text("Writes this phone's local time to $clockName.", style = MaterialTheme.typography.bodySmall)
            Button(onClick = { confirm = true }, enabled = enabled) {
                Icon(Icons.Default.Schedule, contentDescription = null, modifier = Modifier.size(18.dp))
                Text("  Set to phone time")
            }
            message?.let { Text(it, style = MaterialTheme.typography.bodyMedium) }
        }
    }
    if (confirm) {
        val now = LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"))
        AlertDialog(
            onDismissRequest = { confirm = false },
            title = { Text("Set the clock on $meter?") },
            text = { Text("The meter's time becomes $now. Billing and load profile entries use this clock.") },
            confirmButton = { TextButton(onClick = { confirm = false; onSet() }) { Text("Set clock") } },
            dismissButton = { TextButton(onClick = { confirm = false }) { Text("Cancel") } },
        )
    }
}

/**
 * Attribute values the way Gurux shows them: OBIS for logical names, columns
 * for capture objects, Register values scaled with their unit.
 */
/** An attribute's value as the browser shows it; also used for the values in the object lists. */
internal fun formatValue(classId: Int, attribute: Int, data: DlmsData, scaler: ScalerUnit?, decimals: Int?): String = when {
    attribute == 1 && data is DlmsData.OctetString && data.bytes.size == 6 -> ObisCode.of(data.bytes).toString()
    InterfaceClass.hasScaler(classId) && attribute == ObjectBrowserViewModel.VALUE && scaler != null -> scaler.format(data, decimals)
    classId == InterfaceClass.DISCONNECT_CONTROL && attribute == DisconnectControl.OUTPUT_STATE && data is DlmsData.Bool ->
        if (data.value) "Connected (true)" else "Disconnected (false)"
    classId == InterfaceClass.DISCONNECT_CONTROL && attribute == DisconnectControl.CONTROL_STATE && data is DlmsData.Enum ->
        "${DisconnectControl.controlState(data.value)} (${data.value})"
    classId == InterfaceClass.DISCONNECT_CONTROL && attribute == DisconnectControl.CONTROL_MODE && data is DlmsData.Enum ->
        "Mode ${data.value}"
    classId == InterfaceClass.LIMITER && attribute == 2 -> Limiter.valueDefinition(data) ?: data.display()
    classId == InterfaceClass.LIMITER && (attribute == 6 || attribute == 7) && data is DlmsData.Integer -> "${data.value} s"
    classId == InterfaceClass.LIMITER && attribute == 8 -> Limiter.emergencyProfile(data) ?: data.display()
    classId == InterfaceClass.LIMITER && attribute == 11 -> Limiter.actions(data) ?: data.display()
    InterfaceClass.hasScaler(classId) && attribute == ObjectBrowserViewModel.SCALER_UNIT ->
        ScalerUnit.parse(data)?.let { "$it  (scaler ${it.scaler}, unit ${it.unit})" } ?: data.display()
    classId == InterfaceClass.PROFILE_GENERIC && attribute == ObjectBrowserViewModel.CAPTURE_OBJECTS ->
        runCatching {
            CosemStructures.captureObjects(data).joinToString("\n") { "${it.label} (${it.obis} a${it.attribute})" }
        }.getOrElse { data.display() }
    else -> data.displayNumber(decimals)
}
