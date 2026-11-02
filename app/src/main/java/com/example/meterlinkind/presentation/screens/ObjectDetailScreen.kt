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
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.Schedule
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.FilterChip
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
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
import com.example.meterlinkind.data.protocol.CosemObject
import com.example.meterlinkind.data.protocol.CosemStructures
import com.example.meterlinkind.data.protocol.DlmsData
import com.example.meterlinkind.data.protocol.InterfaceClass
import com.example.meterlinkind.data.protocol.ObisCode
import com.example.meterlinkind.presentation.components.ConnectionStrip
import com.example.meterlinkind.presentation.viewmodel.AttributeState
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
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
    val isProfile = obj.classId == InterfaceClass.PROFILE_GENERIC

    LazyColumn(
        modifier = modifier
            .fillMaxSize()
            .padding(horizontal = 16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        item {
            ConnectionStrip(
                deviceName = connection.selectedDeviceName,
                serialNumber = connection.serialNumber,
                connectionState = connection.connectionState,
                associated = online,
            )
        }
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
        items((1..InterfaceClass.attributeCount(obj.classId)).toList()) { attribute ->
            val access = obj.attributeAccess?.let { AccessMode.label(it[attribute] ?: AccessMode.NONE) }
            when {
                !obj.canRead(attribute) -> AttributeRow(
                    attribute, InterfaceClass.attributeName(obj.classId, attribute), access, null,
                    note = "Not readable by this client", enabled = false,
                ) {}
                isProfile && attribute == ObjectBrowserViewModel.BUFFER -> AttributeRow(
                    attribute, InterfaceClass.attributeName(obj.classId, attribute), access, null,
                    note = "Read it with the buffer reader below", enabled = false,
                ) {}
                else -> AttributeRow(
                    attribute = attribute,
                    name = InterfaceClass.attributeName(obj.classId, attribute),
                    access = access,
                    state = browser.attribute(obj, attribute),
                    enabled = online,
                    format = { formatValue(obj.classId, attribute, it) },
                    onRead = { browserViewModel.read(obj, attribute) },
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
        if (isProfile && obj.canRead(ObjectBrowserViewModel.BUFFER)) {
            item {
                BufferSection(
                    state = browser.buffers[obj.obis] ?: ProfileBufferState(),
                    enabled = online,
                    onRead = { last -> browserViewModel.readBuffer(obj, last) },
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
private fun BufferSection(state: ProfileBufferState, enabled: Boolean, onRead: (Int?) -> Unit) {
    var last by rememberSaveable { mutableStateOf<Int?>(10) }
    Column(verticalArrangement = Arrangement.spacedBy(8.dp), modifier = Modifier.padding(top = 8.dp)) {
        Text("Buffer", style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.SemiBold)
        Text(
            state.entriesInUse?.let { "$it entries stored in the meter" } ?: "Reads the newest entries with selective access.",
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            listOf(10, 50, 100, null).forEach { option ->
                FilterChip(
                    selected = last == option,
                    onClick = { last = option },
                    label = { Text(option?.let { "Last $it" } ?: "All") }
                )
            }
        }
        Button(onClick = { onRead(last) }, enabled = enabled && !state.loading, modifier = Modifier.fillMaxWidth()) {
            if (state.loading) {
                CircularProgressIndicator(modifier = Modifier.size(18.dp), strokeWidth = 2.dp, color = MaterialTheme.colorScheme.onPrimary)
                Text("  ${state.progress ?: "Reading..."}")
            } else {
                Text("Read buffer")
            }
        }
        state.error?.let { Text(it, color = MaterialTheme.colorScheme.error, style = MaterialTheme.typography.bodyMedium) }
        state.rows?.let { rows -> BufferTable(state, rows) }
    }
}

@Composable
private fun BufferTable(state: ProfileBufferState, rows: List<List<DlmsData>>) {
    val width = rows.maxOfOrNull { it.size } ?: 0
    val headers = List(width) { i -> state.columns?.getOrNull(i)?.label ?: "Column ${i + 1}" }
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
                    row.forEach {
                        Text(it.display(), modifier = cell, style = MaterialTheme.typography.bodySmall, fontFamily = FontFamily.Monospace)
                    }
                }
            }
        }
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

/** Attribute values the way Gurux shows them: OBIS for logical names, columns for capture objects. */
private fun formatValue(classId: Int, attribute: Int, data: DlmsData): String = when {
    attribute == 1 && data is DlmsData.OctetString && data.bytes.size == 6 -> ObisCode.of(data.bytes).toString()
    classId == InterfaceClass.PROFILE_GENERIC && attribute == ObjectBrowserViewModel.CAPTURE_OBJECTS ->
        runCatching {
            CosemStructures.captureObjects(data).joinToString("\n") { "${it.label} (${it.obis} a${it.attribute})" }
        }.getOrElse { data.display() }
    else -> data.display()
}
