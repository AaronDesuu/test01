package com.example.meterlinkind.presentation.screens

import android.app.DatePickerDialog
import android.app.TimePickerDialog
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.ArrowDownward
import androidx.compose.material.icons.filled.CalendarMonth
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.PhoneAndroid
import androidx.compose.material.icons.filled.Remove
import androidx.compose.material.icons.filled.Schedule
import androidx.compose.material.icons.filled.Upload
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardCapitalization
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import com.example.meterlinkind.data.protocol.Axdr
import com.example.meterlinkind.data.protocol.DlmsData
import com.example.meterlinkind.data.protocol.ScalerUnit
import com.example.meterlinkind.data.protocol.ValueEditor
import com.example.meterlinkind.data.protocol.ValueTree
import com.example.meterlinkind.data.protocol.toHex
import com.example.meterlinkind.presentation.components.OnlineColor
import com.example.meterlinkind.presentation.viewmodel.AttributeState
import com.example.meterlinkind.presentation.viewmodel.AttributeWrite
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

/**
 * Changes one writable attribute. Its value is read first, so the new one keeps
 * the same type; a structure or array is edited field by field (or as hex).
 * Next shows what will be written, naming the meter; Write sends it and reads
 * the attribute back.
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
    val busy = write?.busy == true
    PopUp(onDismiss = { if (!busy) onDismiss() }) {
        if (current == null && readable) {
            Header(Icons.Default.Edit, Tone.EDIT, attributeName, objectName)
            Text(
                state.error ?: "Reading the current value...",
                color = if (state.error != null) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.onSurfaceVariant
            )
            Buttons {
                SecondaryButton("Cancel", onDismiss)
                PrimaryButton("Read again", enabled = !state.loading, onClick = onRead)
            }
            return@PopUp
        }
        EditFlow(objectName, attributeName, meter, current, scaler, write, format, onWrite, onDismiss)
    }
}

/** Edit, then confirm, then the result; [current] is null for a write-only attribute. */
@Composable
private fun EditFlow(
    objectName: String,
    attributeName: String,
    meter: String,
    current: DlmsData?,
    scaler: ScalerUnit?,
    write: AttributeWrite?,
    format: (DlmsData) -> String,
    onWrite: (DlmsData) -> Unit,
    onDismiss: () -> Unit,
) {
    // The editor follows the value read before editing, not the one read back after the write.
    val before = remember { current }
    var tree by remember { mutableStateOf(before?.let { ValueTree.of(it, scaler) } ?: ValueTree.of(DlmsData.Null)) }
    val grouped = before != null && ValueTree.isGroup(before)
    // Hex is the only way for a write-only attribute; a structure can switch to it.
    var hexMode by remember { mutableStateOf(before == null) }
    var hex by remember { mutableStateOf(before?.let { Axdr.encode(it).toHex().uppercase() } ?: "") }
    // For a structure or array this is the whole-value hex editor.
    val rawEditor = remember { if (before == null) ValueEditor.forUnknown() else ValueEditor.forValue(before) }
    var confirming by remember { mutableStateOf(false) }
    val parsed: Result<DlmsData> = if (hexMode) rawEditor.parse(hex) else tree.build()
    val busy = write?.busy == true
    val done = write != null && !write.busy

    when {
        done && write?.error != null -> {
            Header(Icons.Default.Close, Tone.ERROR, "Not written", "Meter $meter")
            Text(write.error, style = MaterialTheme.typography.bodyMedium)
            before?.let { Text("The value is still ${format(it)}.", style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant) }
            Buttons {
                SecondaryButton("Close", onDismiss)
                PrimaryButton("Try again") { parsed.getOrNull()?.let(onWrite) }
            }
        }
        done -> Written(current, format, onDismiss)
        confirming || busy -> {
            Header(Icons.Default.Upload, Tone.WRITE, "Write to the meter?", "Meter $meter")
            Compare(before?.let(format), parsed.getOrNull()?.let(format).orEmpty())
            Text(
                "$attributeName · $objectName",
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Buttons {
                if (busy) {
                    Row(Modifier.weight(1f).height(46.dp), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.Center) {
                        CircularProgressIndicator(modifier = Modifier.size(18.dp), strokeWidth = 2.dp)
                        Spacer(Modifier.width(10.dp))
                        Text("Writing, then reading back...", style = MaterialTheme.typography.labelLarge)
                    }
                } else {
                    SecondaryButton("Back") { confirming = false }
                    PrimaryButton("Write") { parsed.getOrNull()?.let(onWrite) }
                }
            }
        }
        else -> {
            Header(Icons.Default.Edit, Tone.EDIT, attributeName, objectName)
            before?.let { NowBar(format(it)) }
            if (grouped) {
                SingleChoiceSegmentedButtonRow(Modifier.fillMaxWidth()) {
                    SegmentedButton(
                        selected = !hexMode,
                        onClick = {
                            if (hexMode) {
                                // Back to fields only from hex that is a value of the same shape.
                                rawEditor.parse(hex).getOrNull()?.takeIf(ValueTree::isGroup)?.let { tree = ValueTree.of(it); hexMode = false }
                            }
                        },
                        enabled = !hexMode || rawEditor.parse(hex).getOrNull()?.let(ValueTree::isGroup) == true,
                        shape = SegmentedButtonDefaults.itemShape(0, 2)
                    ) { Text("Fields") }
                    SegmentedButton(
                        selected = hexMode,
                        onClick = {
                            if (!hexMode) {
                                tree.build().getOrNull()?.let { hex = Axdr.encode(it).toHex().uppercase() }
                                hexMode = true
                            }
                        },
                        shape = SegmentedButtonDefaults.itemShape(1, 2)
                    ) { Text("Hex") }
                }
            }
            Column(
                Modifier.heightIn(max = 380.dp).verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                when {
                    hexMode -> HexField(rawEditor, hex, parsed) { hex = it }
                    tree is ValueTree.Group -> GroupEditor(tree as ValueTree.Group, emptyList()) { path, change -> tree = tree.update(path, change) }
                    tree is ValueTree.Leaf -> LeafEditor(tree as ValueTree.Leaf) { text -> tree = tree.update(emptyList()) { ValueTree.Leaf((it as ValueTree.Leaf).editor, text) } }
                    else -> Text("This value has nothing to change.", color = MaterialTheme.colorScheme.onSurfaceVariant)
                }
            }
            if (grouped && !hexMode) parsed.exceptionOrNull()?.message?.let {
                Text(it, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.error)
            }
            Buttons {
                SecondaryButton("Cancel", onDismiss)
                PrimaryButton("Next", enabled = parsed.isSuccess) { confirming = true }
            }
        }
    }
}

// --- Frame -------------------------------------------------------------------------------

private enum class Tone { EDIT, WRITE, ERROR }

@Composable
private fun PopUp(onDismiss: () -> Unit, content: @Composable () -> Unit) {
    Dialog(onDismissRequest = onDismiss, properties = DialogProperties(usePlatformDefaultWidth = false)) {
        Surface(
            shape = RoundedCornerShape(28.dp),
            color = MaterialTheme.colorScheme.surfaceContainerLowest,
            shadowElevation = 6.dp,
            modifier = Modifier.padding(horizontal = 24.dp).widthIn(max = 440.dp).fillMaxWidth()
        ) {
            Column(Modifier.padding(24.dp), verticalArrangement = Arrangement.spacedBy(18.dp)) { content() }
        }
    }
}

/** An icon tile, then a title over a smaller line. */
@Composable
private fun Header(icon: ImageVector, tone: Tone, title: String, subtitle: String) {
    val (tile, tint) = when (tone) {
        Tone.EDIT -> MaterialTheme.colorScheme.primaryContainer to MaterialTheme.colorScheme.primary
        Tone.WRITE -> MaterialTheme.colorScheme.tertiaryContainer to MaterialTheme.colorScheme.onTertiaryContainer
        Tone.ERROR -> MaterialTheme.colorScheme.errorContainer to MaterialTheme.colorScheme.error
    }
    Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(14.dp)) {
        Box(Modifier.size(44.dp).background(tile, RoundedCornerShape(14.dp)), contentAlignment = Alignment.Center) {
            Icon(icon, contentDescription = null, tint = tint, modifier = Modifier.size(22.dp))
        }
        Column {
            Text(title, style = MaterialTheme.typography.titleMedium, fontWeight = FontWeight.Medium)
            Text(subtitle, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }
    }
}

@Composable
private fun Buttons(content: @Composable RowScope.() -> Unit) {
    Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(10.dp), content = content)
}

@Composable
private fun RowScope.SecondaryButton(text: String, onClick: () -> Unit) {
    OutlinedButton(onClick = onClick, modifier = Modifier.weight(1f).height(46.dp)) { Text(text) }
}

@Composable
private fun RowScope.PrimaryButton(text: String, enabled: Boolean = true, onClick: () -> Unit) {
    Button(onClick = onClick, enabled = enabled, modifier = Modifier.weight(1f).height(46.dp)) { Text(text) }
}

/** Short enough to sit side by side; longer values stack. */
private fun isShort(text: String) = text.length <= 14 && '\n' !in text

@Composable
private fun NowBar(value: String) {
    Row(
        Modifier
            .fillMaxWidth()
            .background(MaterialTheme.colorScheme.surfaceContainerLow, RoundedCornerShape(12.dp))
            .padding(horizontal = 14.dp, vertical = 10.dp),
        verticalAlignment = Alignment.Top,
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text("Now", style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        Text(
            value,
            style = MaterialTheme.typography.bodyMedium,
            fontFamily = FontFamily.Monospace,
            textAlign = TextAlign.End,
            maxLines = 4,
            modifier = Modifier.weight(1f)
        )
    }
}

/** Now and New: side by side with an arrow, or stacked when either is long. */
@Composable
private fun Compare(now: String?, new: String) {
    val outline = MaterialTheme.colorScheme.outlineVariant
    if (now != null && isShort(now) && isShort(new)) {
        Row(
            Modifier.fillMaxWidth().border(1.dp, outline, RoundedCornerShape(16.dp)),
            verticalAlignment = Alignment.CenterVertically
        ) {
            ValueCell("NOW", now, Modifier.weight(1f).background(MaterialTheme.colorScheme.surfaceContainerLow, RoundedCornerShape(topStart = 16.dp, bottomStart = 16.dp)), new = false)
            Icon(Icons.AutoMirrored.Filled.ArrowForward, contentDescription = null, tint = MaterialTheme.colorScheme.primary, modifier = Modifier.padding(horizontal = 6.dp).size(22.dp))
            ValueCell("NEW", new, Modifier.weight(1f), new = true)
        }
        return
    }
    Column(Modifier.fillMaxWidth().border(1.dp, outline, RoundedCornerShape(16.dp))) {
        now?.let {
            ValueCell("NOW", it, Modifier.fillMaxWidth().background(MaterialTheme.colorScheme.surfaceContainerLow, RoundedCornerShape(topStart = 16.dp, topEnd = 16.dp)), new = false)
            Icon(Icons.Default.ArrowDownward, contentDescription = null, tint = MaterialTheme.colorScheme.primary, modifier = Modifier.align(Alignment.CenterHorizontally).padding(vertical = 2.dp).size(20.dp))
        }
        ValueCell("NEW", new, Modifier.fillMaxWidth(), new = true)
    }
}

@Composable
private fun ValueCell(label: String, value: String, modifier: Modifier, new: Boolean) {
    Column(modifier.padding(14.dp), verticalArrangement = Arrangement.spacedBy(2.dp)) {
        Text(
            label,
            style = MaterialTheme.typography.labelSmall,
            fontWeight = if (new) FontWeight.Medium else null,
            color = if (new) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.onSurfaceVariant
        )
        Text(
            value,
            fontFamily = FontFamily.Monospace,
            fontSize = if (isShort(value)) 22.sp else 14.sp,
            fontWeight = if (new) FontWeight.Medium else null,
            color = if (new) MaterialTheme.colorScheme.onSurface else MaterialTheme.colorScheme.onSurfaceVariant,
            maxLines = 8,
        )
    }
}

@Composable
private fun Written(current: DlmsData?, format: (DlmsData) -> String, onClose: () -> Unit) {
    Column(Modifier.fillMaxWidth(), horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Box(Modifier.size(56.dp).background(OnlineColor.copy(alpha = 0.18f), CircleShape), contentAlignment = Alignment.Center) {
            Icon(Icons.Default.Check, contentDescription = null, tint = OnlineColor, modifier = Modifier.size(28.dp))
        }
        Text("Written", style = MaterialTheme.typography.titleLarge)
        if (current == null) {
            Text("This attribute can't be read back.", style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.onSurfaceVariant)
        } else {
            // What the meter read back after the write.
            val now = format(current)
            Text(
                now,
                fontFamily = FontFamily.Monospace,
                fontSize = if (isShort(now)) 32.sp else 14.sp,
                fontWeight = FontWeight.Medium,
                textAlign = TextAlign.Center,
                maxLines = 8,
            )
        }
    }
    Buttons { PrimaryButton("Close", onClick = onClose) }
}

// --- Editors -----------------------------------------------------------------------------

private val PHONE_TIME = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss")

/** The single value of a scalar attribute, in the input that suits its type. */
@Composable
private fun LeafEditor(leaf: ValueTree.Leaf, onText: (String) -> Unit) {
    val editor = leaf.editor
    val parsed = editor.parse(leaf.text)
    when (editor.kind) {
        ValueEditor.Kind.NUMBER -> NumberStepper(editor, leaf.text, parsed, onText)
        ValueEditor.Kind.BOOLEAN -> OnOff(leaf.text, onText)
        ValueEditor.Kind.DATE_TIME -> DateTimeEditor(editor, leaf.text, parsed, onText)
        ValueEditor.Kind.BITS -> if (leaf.text.length <= 32) Bits(editor, leaf.text, onText) else Field(editor, leaf.text, parsed, onText)
        ValueEditor.Kind.HEX -> HexField(editor, leaf.text, parsed, onText)
        else -> Field(editor, leaf.text, parsed, onText)
    }
}

@Composable
private fun NumberStepper(editor: ValueEditor, text: String, parsed: Result<DlmsData>, onText: (String) -> Unit) {
    Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text("New value", style = MaterialTheme.typography.labelMedium, color = MaterialTheme.colorScheme.primary)
        Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            StepButton(Icons.Default.Remove, "Decrease") { text.toLongOrNull()?.let { onText((it - 1).toString()) } }
            OutlinedTextField(
                value = text,
                onValueChange = onText,
                singleLine = true,
                isError = parsed.isFailure,
                textStyle = MaterialTheme.typography.headlineSmall.copy(fontFamily = FontFamily.Monospace, textAlign = TextAlign.Center),
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number, autoCorrectEnabled = false),
                shape = RoundedCornerShape(14.dp),
                modifier = Modifier.weight(1f)
            )
            StepButton(Icons.Default.Add, "Increase") { text.toLongOrNull()?.let { onText((it + 1).toString()) } }
        }
        Hint(parsed.exceptionOrNull()?.message ?: listOfNotNull(editor.hint, typeOf(editor)).joinToString(" · "), parsed.isFailure)
    }
}

@Composable
private fun StepButton(icon: ImageVector, description: String, onClick: () -> Unit) {
    OutlinedButton(
        onClick = onClick,
        shape = RoundedCornerShape(14.dp),
        contentPadding = PaddingValues(0.dp),
        modifier = Modifier.size(width = 48.dp, height = 56.dp)
    ) { Icon(icon, contentDescription = description, modifier = Modifier.size(20.dp)) }
}

/** The type in brackets at the end of a label: "Number (long-unsigned)" gives "long-unsigned". */
private fun typeOf(editor: ValueEditor): String? = editor.label.substringAfterLast('(', "").removeSuffix(")").ifEmpty { null }

@Composable
private fun Hint(text: String, error: Boolean) {
    if (text.isEmpty()) return
    Text(
        text,
        style = MaterialTheme.typography.bodySmall,
        color = if (error) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.onSurfaceVariant
    )
}

@Composable
private fun OnOff(text: String, onText: (String) -> Unit) {
    Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text("New value", style = MaterialTheme.typography.labelMedium, color = MaterialTheme.colorScheme.primary)
        SingleChoiceSegmentedButtonRow(Modifier.fillMaxWidth()) {
            SegmentedButton(selected = text == "false", onClick = { onText("false") }, shape = SegmentedButtonDefaults.itemShape(0, 2)) { Text("Off (false)") }
            SegmentedButton(selected = text == "true", onClick = { onText("true") }, shape = SegmentedButtonDefaults.itemShape(1, 2)) { Text("On (true)") }
        }
    }
}

/** Date and time buttons that open the pickers, plus the phone's time; seconds stay as typed. */
@Composable
private fun DateTimeEditor(editor: ValueEditor, text: String, parsed: Result<DlmsData>, onText: (String) -> Unit) {
    val context = LocalContext.current
    val time = runCatching { LocalDateTime.parse(text, PHONE_TIME) }.getOrNull()
    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Text("New date and time", style = MaterialTheme.typography.labelMedium, color = MaterialTheme.colorScheme.primary)
        if (time != null) {
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                PickerButton(Icons.Default.CalendarMonth, time.toLocalDate().toString(), Modifier.weight(1f)) {
                    DatePickerDialog(context, { _, y, m, d -> onText(time.withYear(y).withMonth(m + 1).withDayOfMonth(d).format(PHONE_TIME)) },
                        time.year, time.monthValue - 1, time.dayOfMonth).show()
                }
                PickerButton(Icons.Default.Schedule, "%02d:%02d:%02d".format(time.hour, time.minute, time.second), Modifier.weight(1f)) {
                    TimePickerDialog(context, { _, h, min -> onText(time.withHour(h).withMinute(min).format(PHONE_TIME)) },
                        time.hour, time.minute, true).show()
                }
            }
        }
        // The buttons stand for the value; the field is only for text they can't show.
        if (time == null) Field(editor, text, parsed, onText) else Hint(parsed.exceptionOrNull()?.message.orEmpty(), true)
        TextButton(onClick = { onText(LocalDateTime.now().format(PHONE_TIME)) }) {
            Icon(Icons.Default.PhoneAndroid, contentDescription = null, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(6.dp))
            Text("Use phone time")
        }
    }
}

@Composable
private fun PickerButton(icon: ImageVector, text: String, modifier: Modifier, onClick: () -> Unit) {
    OutlinedButton(
        onClick = onClick,
        shape = RoundedCornerShape(14.dp),
        contentPadding = PaddingValues(horizontal = 10.dp),
        modifier = modifier.height(52.dp)
    ) {
        Icon(icon, contentDescription = null, modifier = Modifier.size(18.dp))
        Spacer(Modifier.width(6.dp))
        Text(text, fontFamily = FontFamily.Monospace, style = MaterialTheme.typography.bodyMedium, maxLines = 1, softWrap = false)
    }
}

/** A tap target per bit, numbered from 0. */
@OptIn(ExperimentalLayoutApi::class)
@Composable
private fun Bits(editor: ValueEditor, text: String, onText: (String) -> Unit) {
    Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text("New value · tap a bit to flip it", style = MaterialTheme.typography.labelMedium, color = MaterialTheme.colorScheme.primary)
        FlowRow(horizontalArrangement = Arrangement.spacedBy(6.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
            text.forEachIndexed { i, c ->
                val on = c == '1'
                Column(
                    Modifier
                        .size(width = 34.dp, height = 44.dp)
                        .background(
                            if (on) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surfaceContainerHigh,
                            RoundedCornerShape(8.dp)
                        )
                        .clickable { onText(text.replaceRange(i, i + 1, if (on) "0" else "1")) },
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center
                ) {
                    Text(c.toString(), fontFamily = FontFamily.Monospace, fontWeight = FontWeight.Medium,
                        color = if (on) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.onSurface)
                    Text("$i", fontSize = 9.sp, color = if (on) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.onSurfaceVariant)
                }
            }
        }
        Hint(editor.hint.orEmpty(), false)
    }
}

/** A plain field; numbers with decimals, text, OBIS codes. */
@Composable
private fun Field(editor: ValueEditor, text: String, parsed: Result<DlmsData>, onText: (String) -> Unit, label: String = editor.label, compact: Boolean = false) {
    OutlinedTextField(
        value = text,
        onValueChange = onText,
        label = { Text(label) },
        singleLine = true,
        isError = parsed.isFailure,
        supportingText = if (compact && parsed.isSuccess) null else {
            { Text(parsed.exceptionOrNull()?.message ?: editor.hint ?: if (editor.kind == ValueEditor.Kind.TEXT) "${text.length} characters" else "") }
        },
        textStyle = MaterialTheme.typography.bodyLarge.let { if (editor.kind == ValueEditor.Kind.TEXT && !editor.label.startsWith("OBIS")) it else it.copy(fontFamily = FontFamily.Monospace) },
        keyboardOptions = KeyboardOptions(
            keyboardType = when (editor.kind) {
                ValueEditor.Kind.NUMBER, ValueEditor.Kind.BITS -> KeyboardType.Number
                ValueEditor.Kind.DECIMAL -> KeyboardType.Decimal
                else -> if (editor.label.startsWith("OBIS")) KeyboardType.Decimal else KeyboardType.Text
            },
            autoCorrectEnabled = false,
        ),
        shape = RoundedCornerShape(14.dp),
        modifier = Modifier.fillMaxWidth()
    )
}

/** Bytes as hex over several lines, with a running byte count. */
@Composable
private fun HexField(editor: ValueEditor, text: String, parsed: Result<DlmsData>, onText: (String) -> Unit) {
    val digits = text.count { !it.isWhitespace() }
    OutlinedTextField(
        value = text,
        onValueChange = onText,
        label = { Text(editor.label) },
        minLines = 3,
        isError = parsed.isFailure && text.isNotEmpty(),
        supportingText = {
            Text(
                parsed.exceptionOrNull()?.message?.takeIf { text.isNotEmpty() }
                    ?: listOfNotNull("${digits / 2} bytes", editor.hint).joinToString(" · ")
            )
        },
        textStyle = MaterialTheme.typography.bodyMedium.copy(fontFamily = FontFamily.Monospace),
        keyboardOptions = KeyboardOptions(capitalization = KeyboardCapitalization.Characters, autoCorrectEnabled = false),
        shape = RoundedCornerShape(14.dp),
        modifier = Modifier.fillMaxWidth()
    )
}

/** A structure's fields or an array's items, each in its own input; arrays can add and remove items. */
@Composable
private fun GroupEditor(group: ValueTree.Group, path: List<Int>, onChange: (List<Int>, (ValueTree) -> ValueTree) -> Unit) {
    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        if (group.items.isEmpty()) {
            Text(if (group.array) "No items." else "No fields.", style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }
        group.items.forEachIndexed { i, item ->
            val name = if (group.array) "Item ${i + 1}" else "Field ${i + 1}"
            when (item) {
                is ValueTree.Group -> Column(
                    Modifier
                        .fillMaxWidth()
                        .background(MaterialTheme.colorScheme.surfaceContainerLow, RoundedCornerShape(14.dp))
                        .padding(start = 12.dp, end = 4.dp, top = 4.dp, bottom = 12.dp),
                    verticalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text(
                            "$name · ${if (item.array) "array of ${item.items.size}" else "structure"}",
                            style = MaterialTheme.typography.labelLarge,
                            modifier = Modifier.weight(1f)
                        )
                        if (group.array) RemoveButton(name) { onChange(path) { removeAt(it, i) } }
                    }
                    Box(Modifier.padding(end = 8.dp)) { GroupEditor(item, path + i, onChange) }
                }
                is ValueTree.Leaf -> Row(verticalAlignment = Alignment.CenterVertically) {
                    Box(Modifier.weight(1f)) {
                        CompactLeaf(name, item) { text -> onChange(path + i) { ValueTree.Leaf((it as ValueTree.Leaf).editor, text) } }
                    }
                    if (group.array) RemoveButton(name) { onChange(path) { removeAt(it, i) } }
                }
                is ValueTree.Fixed -> Text("$name · null", style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
            }
        }
        val template = group.template
        if (group.array && template != null) {
            TextButton(onClick = { onChange(path) { ValueTree.Group(true, (it as ValueTree.Group).items + template, template) } }) {
                Icon(Icons.Default.Add, contentDescription = null, modifier = Modifier.size(18.dp))
                Spacer(Modifier.width(6.dp))
                Text("Add item")
            }
        }
    }
}

private fun removeAt(tree: ValueTree, index: Int): ValueTree {
    val group = tree as ValueTree.Group
    return ValueTree.Group(group.array, group.items.filterIndexed { j, _ -> j != index }, group.template)
}

@Composable
private fun RemoveButton(name: String, onClick: () -> Unit) {
    IconButton(onClick = onClick) {
        Icon(Icons.Default.Delete, contentDescription = "Remove $name", tint = MaterialTheme.colorScheme.onSurfaceVariant)
    }
}

/** One field inside a structure: a switch for booleans, a labelled field for the rest. */
@Composable
private fun CompactLeaf(name: String, leaf: ValueTree.Leaf, onText: (String) -> Unit) {
    val editor = leaf.editor
    val parsed = editor.parse(leaf.text)
    if (editor.kind == ValueEditor.Kind.BOOLEAN) {
        Row(Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
            Text("$name · ${editor.label}", style = MaterialTheme.typography.bodyMedium, modifier = Modifier.weight(1f))
            Switch(checked = leaf.text == "true", onCheckedChange = { onText(it.toString()) })
        }
        return
    }
    val hex = editor.kind == ValueEditor.Kind.HEX
    OutlinedTextField(
        value = leaf.text,
        onValueChange = onText,
        label = { Text("$name · ${typeOf(editor) ?: editor.label}") },
        singleLine = !hex,
        isError = parsed.isFailure,
        supportingText = parsed.exceptionOrNull()?.message?.let { { Text(it) } },
        textStyle = MaterialTheme.typography.bodyMedium.copy(fontFamily = FontFamily.Monospace),
        keyboardOptions = KeyboardOptions(
            keyboardType = when (editor.kind) {
                ValueEditor.Kind.NUMBER, ValueEditor.Kind.BITS -> KeyboardType.Number
                ValueEditor.Kind.DECIMAL -> KeyboardType.Decimal
                else -> KeyboardType.Text
            },
            capitalization = if (hex) KeyboardCapitalization.Characters else KeyboardCapitalization.None,
            autoCorrectEnabled = false,
        ),
        shape = RoundedCornerShape(12.dp),
        modifier = Modifier.fillMaxWidth()
    )
}
