package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.clickable
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
import androidx.compose.material3.ListItem
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
import com.example.meterlinkind.presentation.components.ConnectionStrip
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
import com.example.meterlinkind.presentation.viewmodel.ObjectBrowserViewModel
import java.text.DateFormat
import java.util.Date

/**
 * The meter's objects of one interface class, searchable by OBIS or name,
 * like the Gurux object tree. The list is the one read from the meter.
 */
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
        ConnectionStrip(
            deviceName = connection.selectedDeviceName,
            serialNumber = connection.serialNumber,
            connectionState = connection.connectionState,
            associated = connection.isDlmsConnected,
        )

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
                Text(
                    if (browser.meterSerial == null) {
                        "Connect to a meter, then read its object list. It's kept on this phone for next time."
                    } else {
                        "Read the objects ${browser.meterSerial} offers this client. It's kept on this phone for next time."
                    },
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    textAlign = TextAlign.Center
                )
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

        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(
                "${objects.size} ${InterfaceClass.name(classId)} objects · read " +
                    DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.SHORT).format(Date(list.readAtMillis)),
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
                Text("  Read again")
            }
        }
        browser.listMessage?.let {
            Text(it, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }

        LazyColumn(modifier = Modifier.weight(1f)) {
            items(objects, key = { it.obis.toString() }) { obj ->
                val obis = obj.obis.toString()
                ListItem(
                    headlineContent = { Text(obis, fontFamily = FontFamily.Monospace) },
                    supportingContent = if (obj.name != obis) {
                        { Text(obj.name, style = MaterialTheme.typography.bodySmall) }
                    } else null,
                    trailingContent = { Icon(Icons.AutoMirrored.Filled.KeyboardArrowRight, contentDescription = null) },
                    modifier = Modifier.clickable { onOpen(obj) }
                )
                HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant)
            }
        }
    }
}
