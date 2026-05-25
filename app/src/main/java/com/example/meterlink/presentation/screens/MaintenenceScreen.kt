package com.example.meterlink.presentation.screens

import android.annotation.SuppressLint
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.hilt.lifecycle.viewmodel.compose.hiltViewModel
import com.example.meterlink.presentation.components.CollapsibleCard
import com.example.meterlink.presentation.components.ConfirmationDialog
import com.example.meterlink.presentation.components.DeviceStatusCard
import com.example.meterlink.presentation.components.SectionSubheader
import com.example.meterlink.presentation.viewmodel.MaintenanceViewModel
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@SuppressLint("MissingPermission")
@Composable
fun MaintenanceScreen(
    modifier: Modifier = Modifier,
    connectionViewModel: MeterConnectionViewModel = hiltViewModel(),
    maintenanceViewModel: MaintenanceViewModel = hiltViewModel(),
    onNavigateHome: () -> Unit,
) {
    val connectionState by connectionViewModel.uiState.collectAsState()
    val maintenanceState by maintenanceViewModel.uiState.collectAsState()

    // Single collapse for the actions block so the Operation Log can take more space.
    var actionsExpanded by rememberSaveable { mutableStateOf(true) }

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        DeviceStatusCard(
            deviceName = connectionState.selectedDevice?.name,
            serialNumber = connectionState.serialNumber,
            macAddress = connectionState.selectedDevice?.address,
            connectionState = connectionState.connectionState,
            isOperationInProgress = maintenanceState.isOperationInProgress,
            onDisconnect = { connectionViewModel.disconnect() },
            onNavigateHome = onNavigateHome
        )

        CollapsibleCard(
            title = "Maintenance Actions",
            expanded = actionsExpanded,
            onToggle = { actionsExpanded = !actionsExpanded }
        ) {
            // System Settings
            SectionSubheader("System Settings")
            Button(
                onClick = { maintenanceViewModel.requestSetClock() },
                modifier = Modifier.fillMaxWidth(),
                enabled = !maintenanceState.isOperationInProgress,
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.secondary
                )
            ) {
                Text("SET Clock")
            }

            // Data Reset Operations
            SectionSubheader("Data Reset Operations")
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Button(
                    onClick = { maintenanceViewModel.requestResetBilling() },
                    modifier = Modifier.weight(1f),
                    enabled = !maintenanceState.isOperationInProgress,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Text("CLR Bill")
                }

                Button(
                    onClick = { maintenanceViewModel.requestResetEvent() },
                    modifier = Modifier.weight(1f),
                    enabled = !maintenanceState.isOperationInProgress,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Text("CLR Event")
                }
            }

            Spacer(modifier = Modifier.height(8.dp))

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Button(
                    onClick = { maintenanceViewModel.requestResetLoad() },
                    modifier = Modifier.weight(1f),
                    enabled = !maintenanceState.isOperationInProgress,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Text("CLR Load")
                }

                Button(
                    onClick = { maintenanceViewModel.requestResetAmpere() },
                    modifier = Modifier.weight(1f),
                    enabled = !maintenanceState.isOperationInProgress,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Text("CLR Current")
                }
            }

            Surface(
                modifier = Modifier.fillMaxWidth().padding(top = 12.dp),
                color = MaterialTheme.colorScheme.errorContainer,
                shape = MaterialTheme.shapes.small
            ) {
                Text(
                    text = "⚠️ Warning: Reset operations cannot be undone",
                    modifier = Modifier.padding(8.dp),
                    color = MaterialTheme.colorScheme.onErrorContainer,
                    style = MaterialTheme.typography.bodySmall
                )
            }
        }

        // Operation Log Card — takes remaining space; collapsing the actions
        // card above lets this card grow to fill the freed area.
        Card(modifier = Modifier.fillMaxWidth().weight(1f)) {
            Column(modifier = Modifier.fillMaxSize().padding(16.dp)) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "Operation Log",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.SemiBold
                    )

                    if (maintenanceState.isOperationInProgress) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(20.dp),
                            strokeWidth = 2.dp
                        )
                    }
                }

                HorizontalDivider(modifier = Modifier.padding(vertical = 8.dp))

                Text(
                    text = maintenanceState.lastResponse.ifEmpty { "No operations performed" },
                    style = MaterialTheme.typography.bodySmall,
                    modifier = Modifier.fillMaxSize().verticalScroll(rememberScrollState())
                )
            }
        }
    }

    // Confirmation Dialog
    ConfirmationDialog(
        showDialog = maintenanceState.dialogState.show,
        title = maintenanceState.dialogState.title,
        message = maintenanceState.dialogState.message,
        confirmText = maintenanceState.dialogState.confirmText,
        isDestructive = maintenanceState.dialogState.isDestructive,
        onConfirm = maintenanceState.dialogState.onConfirm,
        onDismiss = { maintenanceViewModel.dismissDialog() }
    )
}