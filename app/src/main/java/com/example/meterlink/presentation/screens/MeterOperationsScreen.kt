package com.example.meterlink.presentation.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.CheckCircle
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import com.example.meterlink.presentation.components.ConfirmationDialog
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@Composable
fun MeterOperationsScreen(
    viewModel: MeterConnectionViewModel,
    modifier: Modifier = Modifier
) {
    val uiState by viewModel.uiState.collectAsState()

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        // Status indicator
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(bottom = 16.dp),
            horizontalArrangement = Arrangement.Center,
            verticalAlignment = Alignment.CenterVertically
        ) {
            if (uiState.isOperationInProgress) {
                CircularProgressIndicator(
                    modifier = Modifier.size(24.dp),
                    strokeWidth = 2.dp
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text("Loading...", style = MaterialTheme.typography.bodyMedium)
            } else {
                Icon(
                    imageVector = Icons.Default.CheckCircle,
                    contentDescription = "Complete",
                    tint = Color.Green,
                    modifier = Modifier.size(24.dp)
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text("Ready", style = MaterialTheme.typography.bodyMedium)
            }
        }

        // Operation buttons
        Text(
            text = "Status & Config",
            style = MaterialTheme.typography.titleMedium,
            modifier = Modifier.padding(bottom = 8.dp)
        )

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            OperationButton(
                "MEASURE",
                Modifier.weight(1f),
                enabled = !uiState.isOperationInProgress
            ) { viewModel.readMeasure() }

            OperationButton(
                "STATE",
                Modifier.weight(1f),
                enabled = !uiState.isOperationInProgress
            ) { viewModel.readState() }
        }

        Spacer(modifier = Modifier.height(16.dp))

        Text(
            text = "Actions",
            style = MaterialTheme.typography.titleMedium,
            modifier = Modifier.padding(bottom = 8.dp)
        )

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            OperationButton(
                "DEMAND",
                Modifier.weight(1f),
                isDestructive = true,
                enabled = !uiState.isOperationInProgress
            ) { viewModel.requestDemandReset() }
        }

        Spacer(modifier = Modifier.height(16.dp))

        Text(
            text = "Data Logs",
            style = MaterialTheme.typography.titleMedium,
            modifier = Modifier.padding(bottom = 8.dp)
        )

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            OperationButton(
                "BILLING",
                Modifier.weight(1f),
                enabled = !uiState.isOperationInProgress
            ) { viewModel.readBilling() }

            OperationButton(
                "EVENT",
                Modifier.weight(1f),
                enabled = !uiState.isOperationInProgress
            ) { viewModel.readEvent() }
        }

        Spacer(modifier = Modifier.height(8.dp))

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            OperationButton(
                "LOAD",
                Modifier.weight(1f),
                enabled = !uiState.isOperationInProgress
            ) { viewModel.readLoad() }

            OperationButton(
                "AMPERE",
                Modifier.weight(1f),
                enabled = !uiState.isOperationInProgress
            ) { viewModel.readAmpere() }
        }

        Spacer(modifier = Modifier.height(16.dp))

        // Response display
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp)
            ) {
                Text(
                    text = "Response",
                    style = MaterialTheme.typography.titleMedium,
                    modifier = Modifier.padding(bottom = 8.dp)
                )

                Text(
                    text = uiState.lastResponse.ifEmpty { "No data" },
                    style = MaterialTheme.typography.bodySmall,
                    modifier = Modifier
                        .fillMaxSize()
                        .verticalScroll(rememberScrollState())
                )
            }
        }
    }

    // Confirmation dialog
    ConfirmationDialog(
        showDialog = uiState.dialogState.show,
        title = uiState.dialogState.title,
        message = uiState.dialogState.message,
        confirmText = uiState.dialogState.confirmText,
        isDestructive = uiState.dialogState.isDestructive,
        onConfirm = uiState.dialogState.onConfirm,
        onDismiss = { viewModel.dismissDialog() }
    )
}

@Composable
fun OperationButton(
    text: String,
    modifier: Modifier = Modifier,
    isDestructive: Boolean = false,
    enabled: Boolean = true,
    onClick: () -> Unit
) {
    Button(
        onClick = onClick,
        modifier = modifier,
        enabled = enabled,
        colors = ButtonDefaults.buttonColors(
            containerColor = if (isDestructive)
                MaterialTheme.colorScheme.error
            else
                MaterialTheme.colorScheme.primary
        )
    ) {
        Text(text)
    }
}