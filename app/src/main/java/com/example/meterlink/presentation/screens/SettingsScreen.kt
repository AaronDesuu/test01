package com.example.meterlink.presentation.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
    viewModel: MeterConnectionViewModel,
    onBack: () -> Unit
) {
    val settings = remember { viewModel.getSettings() }
    var account by remember { mutableStateOf(settings["account"] ?: "") }
    var password by remember { mutableStateOf(settings["password"] ?: "3030303030303030") }
    var address by remember { mutableStateOf(settings["address"] ?: "41") }
    var rank by remember { mutableStateOf(settings["rank"] ?: "03") }
    var scan by remember { mutableStateOf(settings["scan"] ?: "3000") }
    var tick by remember { mutableStateOf(settings["tick"] ?: "150") }
    var interval by remember { mutableStateOf(settings["interval"] ?: "100") }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Settings") },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Text("←")
                    }
                }
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(16.dp)
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(
                "Authentication Configuration",
                style = MaterialTheme.typography.titleMedium
            )

            OutlinedTextField(
                value = account,
                onValueChange = { if (it.length <= 8) account = it },
                label = { Text("Account") },
                modifier = Modifier.fillMaxWidth(),
                supportingText = { Text("${account.length}/8 chars max") }
            )

            OutlinedTextField(
                value = password,
                onValueChange = { if (it.length <= 32) password = it },
                label = { Text("Password/Global Key (hex)") },
                modifier = Modifier.fillMaxWidth(),
                supportingText = { Text("${password.length/2.0}/16.0 bytes max") }
            )

            OutlinedTextField(
                value = address,
                onValueChange = { address = it },
                label = { Text("Logical Address (hex)") },
                modifier = Modifier.fillMaxWidth()
            )

            OutlinedTextField(
                value = rank,
                onValueChange = { rank = it },
                label = { Text("Rank") },
                modifier = Modifier.fillMaxWidth(),
                supportingText = { Text("00=Super, 01=Admin, 03=Reader") }
            )

            Spacer(modifier = Modifier.height(8.dp))

            Text(
                "Timing Configuration",
                style = MaterialTheme.typography.titleMedium
            )

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                OutlinedTextField(
                    value = scan,
                    onValueChange = { scan = it },
                    label = { Text("Scan") },
                    modifier = Modifier.weight(1f),
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
                )

                OutlinedTextField(
                    value = tick,
                    onValueChange = { tick = it },
                    label = { Text("Tick") },
                    modifier = Modifier.weight(1f),
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
                )
            }

            OutlinedTextField(
                value = interval,
                onValueChange = { interval = it },
                label = { Text("Interval") },
                modifier = Modifier.fillMaxWidth(),
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
            )

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Button(
                    onClick = {
                        account = ""
                        password = "3030303030303030"
                        address = "41"
                        rank = "03"
                        scan = "3000"
                        tick = "150"
                        interval = "100"
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
                        viewModel.updateSettings(account, password, address, rank, scan, tick, interval)
                        onBack()
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Save Settings")
                }
            }
        }
    }
}