package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.layout.Arrangement
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
    var authPassword by rememberSaveable { mutableStateOf(saved.authPassword) }
    var blockCipherKey by rememberSaveable { mutableStateOf(saved.blockCipherKey) }
    var authenticationKey by rememberSaveable { mutableStateOf(saved.authenticationKey) }
    var broadcastKey by rememberSaveable { mutableStateOf(saved.broadcastKey) }
    val edited = ClientSecurity(authPassword, blockCipherKey, authenticationKey, broadcastKey)

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
        Text(
            "Each value is ${ClientSecurity.KEY_LENGTH} ASCII characters. " +
                if (uiState.isDlmsConnected) "Changes apply from the next connection." else "",
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )

        KeyField("Authentication Password", authPassword) { authPassword = it }
        KeyField("Block Cipher Key", blockCipherKey) { blockCipherKey = it }
        KeyField("Authentication Key", authenticationKey) { authenticationKey = it }
        KeyField("Broadcast Key", broadcastKey) { broadcastKey = it }

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Button(
                onClick = {
                    authPassword = ClientSecurity.DEFAULT
                    blockCipherKey = ClientSecurity.DEFAULT
                    authenticationKey = ClientSecurity.DEFAULT
                    broadcastKey = ClientSecurity.DEFAULT
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

@Composable
private fun KeyField(label: String, value: String, onValueChange: (String) -> Unit) {
    OutlinedTextField(
        value = value,
        onValueChange = { if (it.length <= ClientSecurity.KEY_LENGTH) onValueChange(it) },
        label = { Text(label) },
        singleLine = true,
        isError = !ClientSecurity.isValidKey(value),
        textStyle = MaterialTheme.typography.bodyLarge.copy(fontFamily = FontFamily.Monospace),
        supportingText = { Text("${value.length}/${ClientSecurity.KEY_LENGTH}") },
        modifier = Modifier.fillMaxWidth()
    )
}
