package com.example.meterlink.presentation.screens

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MeterOperationsScreen(
    viewModel: MeterConnectionViewModel,
    onBack: () -> Unit
) {
    val uiState by viewModel.uiState.collectAsState()

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Meter Operations") },
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
        ) {
            // First row
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                OperationButton("MEASURE", Modifier.weight(1f)) { viewModel.readMeasure() }
                OperationButton("STATE", Modifier.weight(1f)) { viewModel.readState() }
                OperationButton("DEMAND", Modifier.weight(1f)) { /* TODO */ }
            }

            Spacer(modifier = Modifier.height(8.dp))

            // Second row
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                OperationButton("BILLING", Modifier.weight(1f)) { /* TODO */ }
                OperationButton("EVENT", Modifier.weight(1f)) { /* TODO */ }
                OperationButton("LOAD", Modifier.weight(1f)) { /* TODO */ }
                OperationButton("AMPERE", Modifier.weight(1f)) { /* TODO */ }
            }

            if (uiState.lastResponse.isNotEmpty()) {
                Card(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(top = 16.dp)
                        .weight(1f)
                ) {
                    Text(
                        text = uiState.lastResponse,
                        modifier = Modifier.padding(16.dp),
                        style = MaterialTheme.typography.bodyMedium
                    )
                }
            }
        }
    }
}

@Composable
fun OperationButton(text: String, modifier: Modifier = Modifier, onClick: () -> Unit) {
    Button(
        onClick = onClick,
        modifier = modifier,
        colors = ButtonDefaults.buttonColors(
            containerColor = MaterialTheme.colorScheme.secondary
        )
    ) {
        Text(text, style = MaterialTheme.typography.labelLarge)
    }
}