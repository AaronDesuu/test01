package com.example.meterlink.presentation.screens

import android.annotation.SuppressLint
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.material3.HorizontalDivider
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlink.presentation.components.ConfirmationDialog
import com.example.meterlink.presentation.components.DeviceStatusCard
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@SuppressLint("MissingPermission")
@Composable
fun MeterOperationsScreen(
    viewModel: MeterConnectionViewModel,
    modifier: Modifier = Modifier,
    onNavigateHome: () -> Unit
) {
    val uiState by viewModel.uiState.collectAsState()

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        DeviceStatusCard(
            deviceName = uiState.selectedDevice?.name,
            serialNumber = uiState.serialNumber,
            macAddress = uiState.selectedDevice?.address,
            connectionState = uiState.connectionState,
            isOperationInProgress = uiState.isOperationInProgress,
            onDisconnect = { viewModel.disconnect() },
            onNavigateHome = onNavigateHome
        )

        // Status & Config Card - Compact
        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp)) {
                Text(
                    text = "Status & Config",
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(bottom = 8.dp)
                )
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = { viewModel.readMeasure() },
                        modifier = Modifier.weight(1f).height(40.dp),
                        enabled = !uiState.isOperationInProgress,
                        contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp)
                    ) {
                        Text("MEASURE", style = MaterialTheme.typography.labelMedium)
                    }
                    Button(
                        onClick = { viewModel.readState() },
                        modifier = Modifier.weight(1f).height(40.dp),
                        enabled = !uiState.isOperationInProgress,
                        contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp)
                    ) {
                        Text("STATE", style = MaterialTheme.typography.labelMedium)
                    }
                }
            }
        }

        // Actions Card - Compact
        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp)) {
                Text(
                    text = "Actions",
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(bottom = 8.dp)
                )
                Button(
                    onClick = { viewModel.requestDemandReset() },
                    modifier = Modifier.fillMaxWidth().height(40.dp),
                    enabled = !uiState.isOperationInProgress,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.error
                    ),
                    contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp)
                ) {
                    Text("DEMAND RESET", style = MaterialTheme.typography.labelMedium)
                }
            }
        }

        // Data Logs Card - Compact
        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp)) {
                Text(
                    text = "Data Logs",
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(bottom = 8.dp)
                )
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = { viewModel.readBilling() },
                        modifier = Modifier.weight(1f).height(40.dp),
                        enabled = !uiState.isOperationInProgress,
                        contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp)
                    ) {
                        Text("BILLING", style = MaterialTheme.typography.labelMedium)
                    }
                    Button(
                        onClick = { viewModel.readEvent() },
                        modifier = Modifier.weight(1f).height(40.dp),
                        enabled = !uiState.isOperationInProgress,
                        contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp)
                    ) {
                        Text("EVENT", style = MaterialTheme.typography.labelMedium)
                    }
                }
                Spacer(modifier = Modifier.height(4.dp))
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = { viewModel.readLoad() },
                        modifier = Modifier.weight(1f).height(40.dp),
                        enabled = !uiState.isOperationInProgress,
                        contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp)
                    ) {
                        Text("LOAD", style = MaterialTheme.typography.labelMedium)
                    }
                    Button(
                        onClick = { viewModel.readAmpere() },
                        modifier = Modifier.weight(1f).height(40.dp),
                        enabled = !uiState.isOperationInProgress,
                        contentPadding = PaddingValues(horizontal = 8.dp, vertical = 4.dp)
                    ) {
                        Text("AMPERE", style = MaterialTheme.typography.labelMedium)
                    }
                }
            }
        }

        // Response Log Card - Takes remaining space
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(12.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "Operation Log",
                        style = MaterialTheme.typography.titleSmall,
                        fontWeight = FontWeight.SemiBold
                    )
                    if (uiState.isOperationInProgress) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(16.dp),
                            strokeWidth = 2.dp
                        )
                    }
                }
                HorizontalDivider(
                    modifier = Modifier.padding(vertical = 8.dp),
                    thickness = DividerDefaults.Thickness,
                    color = DividerDefaults.color
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