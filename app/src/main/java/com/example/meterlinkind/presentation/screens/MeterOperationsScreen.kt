package com.example.meterlinkind.presentation.screens

import android.annotation.SuppressLint
import android.util.Log
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.SaveAlt
import androidx.compose.material3.*
import androidx.compose.material3.HorizontalDivider
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.export.CsvExporter
import com.example.meterlinkind.data.export.OperationType
import com.example.meterlinkind.presentation.components.CollapsibleCard
import com.example.meterlinkind.presentation.components.ConfirmationDialog
import com.example.meterlinkind.presentation.components.DeviceStatusCard
import com.example.meterlinkind.presentation.components.SectionSubheader
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

@SuppressLint("MissingPermission")
@Composable
fun MeterOperationsScreen(
    viewModel: MeterConnectionViewModel,
    modifier: Modifier = Modifier,
    onNavigateHome: () -> Unit
) {
    val uiState by viewModel.uiState.collectAsState()
    val context = LocalContext.current

    // Single collapse for all action sections, so the user can enlarge the
    // Operation Log card by hiding the controls in one tap.
    var actionsExpanded by rememberSaveable { mutableStateOf(true) }

    // Snapshot the state at the moment the user tapped "Save Log" so the write uses
    // the log that was on screen, even if the user starts another read while the
    // SAF document picker is open.
    var pendingSave by remember { mutableStateOf<PendingSave?>(null) }

    val saveCsvLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.CreateDocument("text/csv")
    ) { uri ->
        val save = pendingSave
        pendingSave = null
        if (uri == null || save == null) return@rememberLauncherForActivityResult
        try {
            val csv = CsvExporter.export(
                operation = save.operation,
                deviceName = save.deviceName,
                serialNumber = save.serialNumber,
                lines = save.lines,
            )
            context.contentResolver.openOutputStream(uri)?.use { out ->
                out.write(csv.toByteArray(Charsets.UTF_8))
            } ?: error("openOutputStream returned null")
            Toast.makeText(context, "Log saved", Toast.LENGTH_SHORT).show()
        } catch (t: Throwable) {
            Log.e("MeterOperations", "Failed to save log", t)
            Toast.makeText(context, "Save failed: ${t.message}", Toast.LENGTH_LONG).show()
        }
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        DeviceStatusCard(
            deviceName = uiState.selectedDeviceName,
            serialNumber = uiState.serialNumber,
            macAddress = uiState.selectedDevice?.address,
            connectionState = uiState.connectionState,
            isOperationInProgress = uiState.isOperationInProgress,
            onDisconnect = { viewModel.disconnect() },
            onNavigateHome = onNavigateHome
        )

        CollapsibleCard(
            title = "Meter Actions",
            expanded = actionsExpanded,
            onToggle = { actionsExpanded = !actionsExpanded }
        ) {
            // Status & Config
            SectionSubheader("Status & Config")
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

            // Actions
            SectionSubheader("Actions")
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

            // Data Logs
            SectionSubheader("Data Logs")
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
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        if (uiState.isOperationInProgress) {
                            CircularProgressIndicator(
                                modifier = Modifier.size(16.dp),
                                strokeWidth = 2.dp
                            )
                            Spacer(modifier = Modifier.width(8.dp))
                        }
                        val saveEnabled =
                            uiState.lastOperation != null && !uiState.isOperationInProgress
                        TextButton(
                            onClick = {
                                val op = uiState.lastOperation ?: return@TextButton
                                pendingSave = PendingSave(
                                    operation = op,
                                    deviceName = uiState.selectedDeviceName,
                                    serialNumber = uiState.serialNumber,
                                    lines = uiState.lastResponse.split('\n'),
                                )
                                saveCsvLauncher.launch(
                                    defaultFilename(op, uiState.serialNumber)
                                )
                            },
                            enabled = saveEnabled,
                            contentPadding = PaddingValues(horizontal = 8.dp, vertical = 0.dp),
                        ) {
                            Icon(
                                imageVector = Icons.Filled.SaveAlt,
                                contentDescription = null,
                                modifier = Modifier.size(16.dp),
                            )
                            Spacer(modifier = Modifier.width(4.dp))
                            Text("Save Log", style = MaterialTheme.typography.labelMedium)
                        }
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

private data class PendingSave(
    val operation: OperationType,
    val deviceName: String?,
    val serialNumber: String?,
    val lines: List<String>,
)

private fun defaultFilename(operation: OperationType, serial: String?): String {
    val ts = SimpleDateFormat("yyyyMMdd-HHmmss", Locale.US).format(Date())
    val s = serial?.takeIf { it.isNotBlank() } ?: "unknown"
    return "meterlinkind_${s}_${operation.slug}_$ts.csv"
}