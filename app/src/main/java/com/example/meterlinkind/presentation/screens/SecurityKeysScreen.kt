package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.runtime.saveable.listSaver
import androidx.compose.runtime.toMutableStateList
import androidx.compose.ui.text.input.KeyboardCapitalization
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.model.ClientSecurity
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel

/**
 * Shows and edits the logged-in client's password and keys. Shown right after
 * login, and from the drawer; changes apply from the next association.
 */
@Composable
fun SecurityKeysScreen(
    viewModel: MeterConnectionViewModel,
    onDone: () -> Unit,
    modifier: Modifier = Modifier
) {
    val uiState by viewModel.uiState.collectAsState()
    val saved = remember { viewModel.getClientSecurity() }
    val savedHex = listOf(saved.authPassword, saved.blockCipherKey, saved.authenticationKey, saved.broadcastKey)
    // Open in ASCII when every saved key can be shown that way, otherwise in hex.
    var hexMode by rememberSaveable {
        mutableStateOf(savedHex.any { ClientSecurity.hexToAsciiOrNull(it) == null })
    }
    // What each field shows, in the current mode.
    val texts = rememberSaveable(
        saver = listSaver(save = { it.toList() }, restore = { it.toMutableStateList() })
    ) {
        savedHex.map { if (hexMode) it else ClientSecurity.hexToAsciiOrNull(it)!! }.toMutableStateList()
    }
    val hexValues = texts.map { if (hexMode) it.uppercase() else asciiAsHexOrNull(it) }
    val edited = ClientSecurity(
        hexValues[0] ?: "", hexValues[1] ?: "", hexValues[2] ?: "", hexValues[3] ?: ""
    )
    // ASCII can only show complete, printable bytes.
    val asciiPossible = hexMode && texts.all { ClientSecurity.hexToAsciiOrNull(it) != null }

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp)
            .verticalScroll(rememberScrollState()),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            "Client: ${uiState.client?.label ?: "-"}",
            style = MaterialTheme.typography.titleMedium
        )
        SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
            SegmentedButton(
                selected = !hexMode,
                onClick = {
                    if (hexMode && asciiPossible) {
                        texts.indices.forEach { texts[it] = ClientSecurity.hexToAsciiOrNull(texts[it])!! }
                        hexMode = false
                    }
                },
                enabled = !hexMode || asciiPossible,
                shape = SegmentedButtonDefaults.itemShape(0, 2)
            ) { Text("ASCII (${ClientSecurity.ASCII_LENGTH})") }
            SegmentedButton(
                selected = hexMode,
                onClick = {
                    if (!hexMode) {
                        texts.indices.forEach { texts[it] = ClientSecurity.asciiToHex(texts[it]) }
                        hexMode = true
                    }
                },
                shape = SegmentedButtonDefaults.itemShape(1, 2)
            ) { Text("Hex (${ClientSecurity.HEX_LENGTH})") }
        }
        Text(
            (if (hexMode) {
                "Each value is ${ClientSecurity.HEX_LENGTH} hex digits (${ClientSecurity.KEY_BYTES} bytes)." +
                    if (!asciiPossible) " ASCII needs complete, printable bytes in every field." else ""
            } else {
                "Each value is ${ClientSecurity.ASCII_LENGTH} ASCII characters."
            }) + if (uiState.isDlmsConnected) " Changes apply from the next connection." else "",
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )

        KEY_LABELS.forEachIndexed { i, label ->
            KeyField(label, texts[i], hexMode) { texts[i] = it }
        }

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Button(
                onClick = {
                    texts.indices.forEach {
                        texts[it] = if (hexMode) ClientSecurity.DEFAULT
                        else ClientSecurity.hexToAsciiOrNull(ClientSecurity.DEFAULT)!!
                    }
                },
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.secondary
                )
            ) {
                Text("Reset to Default")
            }

            Button(
                onClick = {
                    viewModel.saveClientSecurity(edited)
                    onDone()
                },
                enabled = edited.isValid,
                modifier = Modifier.weight(1f)
            ) {
                Text("Save & Continue")
            }
        }
    }
}

private val KEY_LABELS = listOf("Authentication Password", "Block Cipher Key", "Authentication Key", "Broadcast Key")

/** The hex of a complete ASCII key, or null while it's incomplete or invalid. */
private fun asciiAsHexOrNull(text: String): String? =
    if (ClientSecurity.isValidAscii(text)) ClientSecurity.asciiToHex(text) else null

/**
 * One key in the current mode. ASCII takes 16 printable characters; hex takes 32
 * digits in either case (anything else is ignored; saved upper-case). Under the
 * field: the same key in the other form, and the count.
 */
@Composable
private fun KeyField(label: String, value: String, hexMode: Boolean, onValueChange: (String) -> Unit) {
    val max = if (hexMode) ClientSecurity.HEX_LENGTH else ClientSecurity.ASCII_LENGTH
    val valid = if (hexMode) ClientSecurity.isValidHex(value) else ClientSecurity.isValidAscii(value)
    val other = if (hexMode) {
        ClientSecurity.hexToAsciiOrNull(value)?.let { "ASCII: $it" }
            ?: if (valid) "ASCII: not printable" else ""
    } else {
        if (value.isEmpty()) "" else "Hex: ${ClientSecurity.asciiToHex(value)}"
    }
    OutlinedTextField(
        value = value,
        onValueChange = { input ->
            // Only drops what isn't a hex digit; changing case while typing confuses the keyboard.
            val cleaned = if (hexMode) input.filter { it.uppercaseChar() in "0123456789ABCDEF" } else input
            if (cleaned.length <= max) onValueChange(cleaned)
        },
        label = { Text(label) },
        singleLine = true,
        isError = !valid,
        textStyle = MaterialTheme.typography.bodyLarge.copy(fontFamily = FontFamily.Monospace),
        keyboardOptions = KeyboardOptions(
            capitalization = if (hexMode) KeyboardCapitalization.Characters else KeyboardCapitalization.None,
            autoCorrectEnabled = false
        ),
        supportingText = {
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                Text(other, fontFamily = FontFamily.Monospace)
                Text("${value.length}/$max")
            }
        },
        modifier = Modifier.fillMaxWidth()
    )
}
