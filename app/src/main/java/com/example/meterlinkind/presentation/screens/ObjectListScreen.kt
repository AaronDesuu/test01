package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Surface
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import com.example.meterlinkind.data.protocol.DlmsData
import com.example.meterlinkind.presentation.viewmodel.AttributeState
import com.example.meterlinkind.presentation.viewmodel.ValuesReadProgress
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.KeyboardArrowRight
import androidx.compose.material.icons.filled.Clear
import androidx.compose.material.icons.filled.Download
import androidx.compose.material.icons.filled.Search
import androidx.compose.material.icons.filled.Sync
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.protocol.CosemObject
import com.example.meterlinkind.data.protocol.InterfaceClass
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
import com.example.meterlinkind.presentation.viewmodel.ObjectBrowserViewModel
import java.text.DateFormat
import java.util.Date

/** The objects of one interface class in the list read from the meter, searchable by OBIS or name. */
@Composable
fun ObjectListScreen(
    classId: Int,
    connectionViewModel: MeterConnectionViewModel,
    browserViewModel: ObjectBrowserViewModel,
    onOpen: (CosemObject) -> Unit,
    modifier: Modifier = Modifier,
) {
    val connection by connectionViewModel.uiState.collectAsState()
    val browser by browserViewModel.uiState.collectAsState()
    var query by rememberSaveable(classId) { mutableStateOf("") }
    val objects = remember(classId, browser.objectList, query) {
        browserViewModel.objects(classId).filter {
            query.isBlank() || it.name.contains(query, ignoreCase = true) || it.obis.toString().contains(query.trim())
        }
    }
    val canRead = connection.isDlmsConnected && browser.meterSerial != null && !browser.readingList

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(horizontal = 16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {

        val list = browser.objectList
        if (list == null) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 32.dp),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Text("No object list for this meter yet", style = MaterialTheme.typography.titleMedium)
                Button(onClick = browserViewModel::readObjectList, enabled = canRead) {
                    if (browser.readingList) {
                        CircularProgressIndicator(modifier = Modifier.size(18.dp), strokeWidth = 2.dp, color = MaterialTheme.colorScheme.onPrimary)
                    } else {
                        Icon(Icons.Default.Download, contentDescription = null, modifier = Modifier.size(18.dp))
                    }
                    Text("  Read from meter")
                }
                browser.listMessage?.let {
                    Text(it, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant, textAlign = TextAlign.Center)
                }
            }
            return@Column
        }

        OutlinedTextField(
            value = query,
            onValueChange = { query = it },
            placeholder = { Text("Search OBIS or name") },
            leadingIcon = { Icon(Icons.Default.Search, contentDescription = null) },
            trailingIcon = {
                if (query.isNotEmpty()) {
                    IconButton(onClick = { query = "" }) { Icon(Icons.Default.Clear, contentDescription = "Clear search") }
                }
            },
            singleLine = true,
            modifier = Modifier.fillMaxWidth()
        )

        val showValues = classId in VALUE_CLASSES
        val valuesRead = if (showValues) objects.count { browser.attribute(it, ObjectBrowserViewModel.VALUE).value != null } else 0
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(
                buildString {
                    append("${objects.size} objects")
                    if (showValues) append(" · $valuesRead read")
                    append(" · list from ")
                    append(DateFormat.getDateInstance(DateFormat.MEDIUM).format(Date(list.readAtMillis)))
                },
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.weight(1f)
            )
            TextButton(onClick = browserViewModel::readObjectList, enabled = canRead) {
                if (browser.readingList) {
                    CircularProgressIndicator(modifier = Modifier.size(16.dp), strokeWidth = 2.dp)
                } else {
                    Icon(Icons.Default.Sync, contentDescription = null, modifier = Modifier.size(18.dp))
                }
                Text("  Read list again")
            }
        }
        browser.listMessage?.let {
            Text(it, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }

        val online = connection.isDlmsConnected
        val listState = rememberLazyListState()
        Box(modifier = Modifier.weight(1f)) {
            LazyColumn(
                state = listState,
                // Room for the read bar under the last row.
                contentPadding = PaddingValues(bottom = if (showValues && online) 84.dp else 8.dp),
                modifier = Modifier.fillMaxSize()
            ) {
                items(objects, key = { it.obis.toString() }) { obj ->
                    ObjectRow(
                        obj = obj,
                        value = if (showValues) browser.attribute(obj, ObjectBrowserViewModel.VALUE) else null,
                        format = { formatValue(obj.classId, ObjectBrowserViewModel.VALUE, it, browser.scalers[obj.obis], browser.decimals) },
                        canRead = online && obj.canRead(ObjectBrowserViewModel.VALUE),
                        onRead = { browserViewModel.read(obj, ObjectBrowserViewModel.VALUE) },
                        onOpen = { onOpen(obj) },
                    )
                    HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant)
                }
            }
            if (showValues && online && objects.isNotEmpty()) {
                // The objects in the (filtered) list whose value this client may read.
                val readable = remember(objects) { objects.filter { it.canRead(ObjectBrowserViewModel.VALUE) } }
                ReadValuesBar(
                    count = readable.size,
                    progress = browser.valuesRead,
                    onRead = { browserViewModel.readValues(readable) },
                    onStop = browserViewModel::stopReadingValues,
                    modifier = Modifier
                        .align(Alignment.BottomCenter)
                        .padding(bottom = 12.dp)
                )
            }
        }
    }
}

/** Classes whose attribute 2 is a value worth showing in the list. */
private val VALUE_CLASSES = setOf(
    InterfaceClass.DATA, InterfaceClass.REGISTER, InterfaceClass.EXTENDED_REGISTER,
    InterfaceClass.CLOCK, InterfaceClass.DISCONNECT_CONTROL,
)

/**
 * One object: name, OBIS code under it, and on the right its last value with
 * the time it was read, or a Read button. [value] is null for classes that
 * show no value; the row then ends in an arrow.
 */
@Composable
private fun ObjectRow(
    obj: CosemObject,
    value: AttributeState?,
    format: (DlmsData) -> String,
    canRead: Boolean,
    onRead: () -> Unit,
    onOpen: () -> Unit,
) {
    val obis = obj.obis.toString()
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onOpen)
            .padding(start = 4.dp, end = 4.dp, top = 10.dp, bottom = 10.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(
                obj.name,
                style = MaterialTheme.typography.bodyLarge,
                fontWeight = FontWeight.Medium,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
            if (obj.name != obis) {
                Text(
                    obis,
                    style = MaterialTheme.typography.bodySmall,
                    fontFamily = FontFamily.Monospace,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
        when {
            value == null -> Icon(Icons.AutoMirrored.Filled.KeyboardArrowRight, contentDescription = null)
            value.loading -> CircularProgressIndicator(modifier = Modifier.padding(horizontal = 12.dp).size(20.dp), strokeWidth = 2.dp)
            value.value != null -> Column(
                horizontalAlignment = Alignment.End,
                modifier = Modifier.widthIn(max = 170.dp)
            ) {
                Text(
                    format(value.value),
                    style = MaterialTheme.typography.titleMedium,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                value.readAt?.let {
                    Text(
                        TIME_FORMAT.format(Date(it)),
                        style = MaterialTheme.typography.labelSmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
            value.error != null -> TextButton(onClick = onRead, enabled = canRead) {
                Text("Failed · retry", color = MaterialTheme.colorScheme.error, style = MaterialTheme.typography.labelLarge)
            }
            canRead -> TextButton(onClick = onRead) {
                Icon(Icons.Default.Download, contentDescription = null, modifier = Modifier.size(16.dp))
                Text(" Read", style = MaterialTheme.typography.labelLarge)
            }
            else -> Icon(Icons.AutoMirrored.Filled.KeyboardArrowRight, contentDescription = null)
        }
    }
}

/** Reads the values of every object in the list; while it runs, shows progress and Stop. */
@Composable
private fun ReadValuesBar(
    count: Int,
    progress: ValuesReadProgress?,
    onRead: () -> Unit,
    onStop: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val reading = progress != null
    Surface(
        onClick = if (reading) onStop else onRead,
        enabled = reading || count > 0,
        shape = RoundedCornerShape(16.dp),
        color = MaterialTheme.colorScheme.primary,
        contentColor = MaterialTheme.colorScheme.onPrimary,
        shadowElevation = 6.dp,
        modifier = modifier.fillMaxWidth()
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 14.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            if (reading) {
                CircularProgressIndicator(
                    modifier = Modifier.size(18.dp),
                    strokeWidth = 2.dp,
                    color = MaterialTheme.colorScheme.onPrimary
                )
                Text(
                    if (progress!!.stopping) "Stopping..." else "Reading ${progress.done + 1} of ${progress.total}",
                    style = MaterialTheme.typography.labelLarge,
                    modifier = Modifier.weight(1f)
                )
                if (!progress.stopping) {
                    Text("Stop", style = MaterialTheme.typography.labelLarge, fontWeight = FontWeight.Bold)
                }
            } else {
                Icon(Icons.Default.Download, contentDescription = null, modifier = Modifier.size(18.dp))
                Text("Read all values", style = MaterialTheme.typography.labelLarge, modifier = Modifier.weight(1f))
                Text(
                    if (count == 1) "1 object" else "$count objects",
                    style = MaterialTheme.typography.labelMedium
                )
            }
        }
    }
}

private val TIME_FORMAT = DateFormat.getTimeInstance(DateFormat.MEDIUM)
