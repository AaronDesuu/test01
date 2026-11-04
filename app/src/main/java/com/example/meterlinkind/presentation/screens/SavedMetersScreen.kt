package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Speed
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.ListItem
import androidx.compose.material3.ListItemDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.repository.SavedMeter
import com.example.meterlinkind.presentation.viewmodel.ObjectBrowserViewModel
import java.text.DateFormat
import java.util.Date

/** The object lists kept on the phone, one per meter and client, with delete. */
@Composable
fun SavedMetersScreen(browserViewModel: ObjectBrowserViewModel, modifier: Modifier = Modifier) {
    var meters by remember { mutableStateOf(browserViewModel.savedMeters()) }
    var confirmAll by remember { mutableStateOf(false) }

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(horizontal = 16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Text(
            "Object lists read from each meter, kept per client. A deleted list is read again " +
                "from the meter next time (about 18 s).",
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        if (meters.isEmpty()) {
            Text("Nothing saved yet.", style = MaterialTheme.typography.bodyMedium, modifier = Modifier.padding(vertical = 24.dp))
        }
        LazyColumn(modifier = Modifier.weight(1f, fill = false), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            items(meters, key = { "${it.serial}_${it.client}" }) { meter ->
                ListItem(
                    leadingContent = { Icon(Icons.Default.Speed, contentDescription = null) },
                    headlineContent = { Text(meter.serial, fontFamily = FontFamily.Monospace) },
                    supportingContent = {
                        Text(
                            "${meter.client.label} · ${meter.objectCount} objects\nRead " +
                                DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.SHORT).format(Date(meter.readAtMillis))
                        )
                    },
                    trailingContent = {
                        IconButton(onClick = {
                            browserViewModel.deleteSavedMeter(meter)
                            meters = browserViewModel.savedMeters()
                        }) {
                            Icon(Icons.Default.Delete, contentDescription = "Delete ${meter.serial} (${meter.client.label})")
                        }
                    },
                    colors = ListItemDefaults.colors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow),
                    modifier = Modifier.clip(RoundedCornerShape(16.dp))
                )
            }
        }
        if (meters.isNotEmpty()) {
            FilledTonalButton(onClick = { confirmAll = true }, modifier = Modifier.fillMaxWidth()) {
                Text("Delete all saved meters")
            }
        }
    }

    if (confirmAll) {
        AlertDialog(
            onDismissRequest = { confirmAll = false },
            title = { Text("Delete all saved meters?") },
            text = { Text("Every object list on this phone is removed. Each meter's list is read again on its next use.") },
            confirmButton = {
                TextButton(onClick = {
                    browserViewModel.deleteAllSavedMeters()
                    meters = emptyList<SavedMeter>()
                    confirmAll = false
                }) { Text("Delete all") }
            },
            dismissButton = { TextButton(onClick = { confirmAll = false }) { Text("Cancel") } },
        )
    }
}
