package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.input.KeyboardCapitalization
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.protocol.DlmsData
import com.example.meterlinkind.data.protocol.ScalerUnit
import com.example.meterlinkind.data.protocol.ValueEditor
import com.example.meterlinkind.presentation.viewmodel.AttributeState
import com.example.meterlinkind.presentation.viewmodel.AttributeWrite
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

/**
 * Changes one writable attribute. Its value is read first, so the new one keeps
 * the same type. Next shows what will be written, naming the meter; Write sends
 * it and reads the attribute back.
 */
@Composable
fun EditAttributeDialog(
    objectName: String,
    attributeName: String,
    meter: String,
    state: AttributeState,
    /** False for a write-only attribute: there's nothing to read first. */
    readable: Boolean,
    scaler: ScalerUnit?,
    /** This attribute's write once one is under way or done; null before. */
    write: AttributeWrite?,
    format: (DlmsData) -> String,
    onRead: () -> Unit,
    onWrite: (DlmsData) -> Unit,
    onDismiss: () -> Unit,
) {
    val current = state.value
    if (current == null && readable) {
        AlertDialog(
            onDismissRequest = onDismiss,
            icon = { Icon(Icons.Default.Edit, contentDescription = null) },
            title = { Text("Change $attributeName") },
            text = {
                Text(
                    state.error ?: "Reading the current value...",
                    color = if (state.error != null) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.onSurfaceVariant
                )
            },
            confirmButton = { TextButton(onClick = onRead, enabled = !state.loading) { Text("Read again") } },
            dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } },
        )
        return
    }

    // The editor follows the value read before editing, not the one read back after the write.
    val editor = remember(scaler) { current?.let { ValueEditor.forValue(it, scaler) } ?: ValueEditor.forUnknown() }
    val before = remember { current }
    var text by rememberSaveable { mutableStateOf(editor.initial) }
    var confirming by rememberSaveable { mutableStateOf(false) }
    val parsed = editor.parse(text)
    val busy = write?.busy == true
    val done = write != null && !write.busy

    AlertDialog(
        onDismissRequest = { if (!busy) onDismiss() },
        icon = { Icon(Icons.Default.Edit, contentDescription = null) },
        title = { Text("Change $attributeName") },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
                Text(objectName, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
                when {
                    done && write?.error != null -> Text("Not written: ${write.error}", color = MaterialTheme.colorScheme.error)
                    done -> if (current == null) {
                        Text("Written.")
                    } else {
                        Text("Written. The meter now reads:")
                        Text(format(current), style = MaterialTheme.typography.titleMedium)
                    }
                    confirming -> {
                        Text("Write ${parsed.getOrNull()?.let(format)} to $attributeName on meter $meter?")
                        before?.let {
                            Text(
                                "Now: ${format(it)}",
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }
                    else -> ValueField(editor, text, parsed, before, format) { text = it }
                }
            }
        },
        confirmButton = {
            when {
                done -> TextButton(onClick = onDismiss) { Text("Close") }
                busy -> Row(verticalAlignment = Alignment.CenterVertically) {
                    CircularProgressIndicator(modifier = Modifier.size(18.dp), strokeWidth = 2.dp)
                    Text("  Writing...")
                }
                confirming -> TextButton(onClick = { parsed.getOrNull()?.let(onWrite) }) { Text("Write") }
                else -> TextButton(onClick = { confirming = true }, enabled = parsed.isSuccess) { Text("Next") }
            }
        },
        dismissButton = {
            when {
                busy || done -> {}
                confirming -> TextButton(onClick = { confirming = false }) { Text("Back") }
                else -> TextButton(onClick = onDismiss) { Text("Cancel") }
            }
        },
    )
}

private val NOW_FORMAT = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss")

/** The current value, then the field (or switch) for the new one. */
@Composable
private fun ValueField(
    editor: ValueEditor,
    text: String,
    parsed: Result<DlmsData>,
    current: DlmsData?,
    format: (DlmsData) -> String,
    onText: (String) -> Unit,
) {
    current?.let { Text("Now: ${format(it)}", style = MaterialTheme.typography.bodyMedium) }
    if (editor.kind == ValueEditor.Kind.BOOLEAN) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(editor.label, modifier = Modifier.weight(1f))
            Switch(checked = text == "true", onCheckedChange = { onText(it.toString()) })
        }
        return
    }
    val monospace = editor.kind == ValueEditor.Kind.HEX || editor.kind == ValueEditor.Kind.BITS
    OutlinedTextField(
        value = text,
        onValueChange = onText,
        label = { Text(editor.label) },
        singleLine = editor.kind != ValueEditor.Kind.HEX,
        isError = parsed.isFailure,
        supportingText = { Text(parsed.exceptionOrNull()?.message ?: editor.hint.orEmpty()) },
        textStyle = MaterialTheme.typography.bodyLarge.let { if (monospace) it.copy(fontFamily = FontFamily.Monospace) else it },
        keyboardOptions = KeyboardOptions(
            keyboardType = when (editor.kind) {
                ValueEditor.Kind.NUMBER, ValueEditor.Kind.BITS -> KeyboardType.Number
                ValueEditor.Kind.DECIMAL -> KeyboardType.Decimal
                else -> KeyboardType.Text
            },
            capitalization = if (editor.kind == ValueEditor.Kind.HEX) KeyboardCapitalization.Characters else KeyboardCapitalization.None,
            autoCorrectEnabled = false,
        ),
        modifier = Modifier.fillMaxWidth()
    )
    if (editor.kind == ValueEditor.Kind.DATE_TIME) {
        TextButton(onClick = { onText(LocalDateTime.now().format(NOW_FORMAT)) }) { Text("Use phone time") }
    }
}
