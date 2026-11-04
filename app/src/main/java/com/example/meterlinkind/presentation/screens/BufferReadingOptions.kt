package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.CalendarMonth
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.DatePicker
import androidx.compose.material3.DatePickerDialog
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TimePicker
import androidx.compose.material3.rememberDatePickerState
import androidx.compose.material3.rememberTimePickerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.presentation.viewmodel.BufferSelection
import java.time.Instant
import java.time.LocalDate
import java.time.LocalDateTime
import java.time.LocalTime
import java.time.ZoneOffset
import java.time.format.DateTimeFormatter

/** The four ways Gurux reads a buffer; Settings picks the one the reader opens with. */
enum class BufferReadMode(val label: String) {
    ENTRIES("Read index + count"),
    LAST_DAYS("Read last days"),
    RANGE("Read from / to"),
    ALL("All"),
}

private val DATE_TIME = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm")

/**
 * Gurux's "Reading" box for a Profile Generic buffer: Read [index] Count [n],
 * Read last [n] days, Read from [time] to [time], or All.
 */
@Composable
fun BufferReadingOptions(
    defaultMode: BufferReadMode,
    enabled: Boolean,
    loading: Boolean,
    progress: String?,
    onRead: (BufferSelection) -> Unit,
) {
    var mode by rememberSaveable { mutableStateOf(defaultMode) }
    var first by rememberSaveable { mutableStateOf("1") }
    var count by rememberSaveable { mutableStateOf("1") }
    var days by rememberSaveable { mutableStateOf("0") }
    // LocalDateTime.toString(), so the choice survives rotation.
    var from by rememberSaveable { mutableStateOf(LocalDate.now().atStartOfDay().toString()) }
    var to by rememberSaveable { mutableStateOf(LocalDateTime.now().withSecond(0).withNano(0).toString()) }

    val fromTime = LocalDateTime.parse(from)
    val toTime = LocalDateTime.parse(to)
    val selection: BufferSelection? = when (mode) {
        BufferReadMode.ENTRIES -> {
            val f = first.toLongOrNull()?.takeIf { it >= 1 }
            val c = count.toLongOrNull()?.takeIf { it >= 1 }
            if (f != null && c != null) BufferSelection.Entries(f, c) else null
        }
        BufferReadMode.LAST_DAYS -> days.toLongOrNull()?.takeIf { it >= 0 }?.let { BufferSelection.LastDays(it) }
        BufferReadMode.RANGE -> if (!fromTime.isAfter(toTime)) BufferSelection.Range(fromTime, toTime) else null
        BufferReadMode.ALL -> BufferSelection.All
    }
    val problem = when {
        selection != null -> null
        mode == BufferReadMode.ENTRIES -> "Index and count start at 1."
        mode == BufferReadMode.LAST_DAYS -> "Enter 0 or more days."
        else -> "From must not be after To."
    }

    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
        Text("Reading", style = MaterialTheme.typography.labelLarge, color = MaterialTheme.colorScheme.onSurfaceVariant)

        ModeRow("Read", mode == BufferReadMode.ENTRIES, { mode = BufferReadMode.ENTRIES }) {
            NumberField(first, { first = it }, mode == BufferReadMode.ENTRIES)
            Text("Count")
            NumberField(count, { count = it }, mode == BufferReadMode.ENTRIES)
        }
        ModeRow("Read last", mode == BufferReadMode.LAST_DAYS, { mode = BufferReadMode.LAST_DAYS }) {
            NumberField(days, { days = it }, mode == BufferReadMode.LAST_DAYS)
            Text(if (days == "0") "days (today)" else "days")
        }
        ModeRow("Read from", mode == BufferReadMode.RANGE, { mode = BufferReadMode.RANGE }) {}
        Column(
            modifier = Modifier.padding(start = 48.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            DateTimeField(fromTime, mode == BufferReadMode.RANGE) { from = it.toString() }
            Text("to", style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
            DateTimeField(toTime, mode == BufferReadMode.RANGE) { to = it.toString() }
        }
        ModeRow("All", mode == BufferReadMode.ALL, { mode = BufferReadMode.ALL }) {}

        problem?.let { Text(it, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.error) }
        Button(
            onClick = { selection?.let(onRead) },
            enabled = enabled && selection != null && !loading,
            modifier = Modifier
                .fillMaxWidth()
                .padding(top = 4.dp)
        ) {
            if (loading) {
                CircularProgressIndicator(modifier = Modifier.size(18.dp), strokeWidth = 2.dp, color = MaterialTheme.colorScheme.onPrimary)
                Text("  ${progress ?: "Reading..."}")
            } else {
                Text("Read buffer")
            }
        }
    }
}

@Composable
private fun ModeRow(label: String, selected: Boolean, onSelect: () -> Unit, controls: @Composable () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .selectable(selected = selected, onClick = onSelect, role = Role.RadioButton),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        RadioButton(selected = selected, onClick = null, modifier = Modifier.padding(start = 12.dp, end = 4.dp))
        Text(label, modifier = Modifier.width(80.dp))
        controls()
    }
}

@Composable
private fun NumberField(value: String, onChange: (String) -> Unit, enabled: Boolean) {
    OutlinedTextField(
        value = value,
        onValueChange = { text -> if (text.length <= 6 && text.all(Char::isDigit)) onChange(text) },
        enabled = enabled,
        singleLine = true,
        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
        modifier = Modifier.width(84.dp)
    )
}

/** A date and time chosen with the Material date picker, then the time picker. */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun DateTimeField(value: LocalDateTime, enabled: Boolean, onChange: (LocalDateTime) -> Unit) {
    var pickDate by remember { mutableStateOf(false) }
    var pickedDate by remember { mutableStateOf<LocalDate?>(null) }

    OutlinedButton(onClick = { pickDate = true }, enabled = enabled) {
        Icon(Icons.Default.CalendarMonth, contentDescription = null, modifier = Modifier.size(18.dp))
        Text("  ${value.format(DATE_TIME)}")
    }

    if (pickDate) {
        // The date picker works in UTC midnights.
        val state = rememberDatePickerState(
            initialSelectedDateMillis = value.toLocalDate().atStartOfDay(ZoneOffset.UTC).toInstant().toEpochMilli()
        )
        DatePickerDialog(
            onDismissRequest = { pickDate = false },
            confirmButton = {
                TextButton(onClick = {
                    pickedDate = state.selectedDateMillis?.let { Instant.ofEpochMilli(it).atZone(ZoneOffset.UTC).toLocalDate() }
                        ?: value.toLocalDate()
                    pickDate = false
                }) { Text("Next") }
            },
            dismissButton = { TextButton(onClick = { pickDate = false }) { Text("Cancel") } },
        ) { DatePicker(state = state) }
    }

    pickedDate?.let { date ->
        val state = rememberTimePickerState(initialHour = value.hour, initialMinute = value.minute, is24Hour = true)
        AlertDialog(
            onDismissRequest = { pickedDate = null },
            title = { Text(date.toString()) },
            text = { TimePicker(state = state) },
            confirmButton = {
                TextButton(onClick = {
                    onChange(LocalDateTime.of(date, LocalTime.of(state.hour, state.minute)))
                    pickedDate = null
                }) { Text("OK") }
            },
            dismissButton = { TextButton(onClick = { pickedDate = null }) { Text("Cancel") } },
        )
    }
}
