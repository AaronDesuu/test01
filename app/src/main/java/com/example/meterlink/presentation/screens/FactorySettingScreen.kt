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
import androidx.hilt.lifecycle.viewmodel.compose.hiltViewModel
import com.example.meterlink.presentation.components.ConfirmationDialog
import com.example.meterlink.presentation.components.DeviceStatusCard
import com.example.meterlink.presentation.viewmodel.FactorySettingViewModel
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@SuppressLint("MissingPermission")
@Composable
fun FactorySettingScreen(
    modifier: Modifier = Modifier,
    connectionViewModel: MeterConnectionViewModel = hiltViewModel(),
    factoryViewModel: FactorySettingViewModel = hiltViewModel(),
    onNavigateHome: () -> Unit
) {
    val connectionState by connectionViewModel.uiState.collectAsState()
    val factoryState by factoryViewModel.uiState.collectAsState()

    Column(
        modifier = modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        DeviceStatusCard(
            deviceName = connectionState.selectedDevice?.name,
            serialNumber = connectionState.serialNumber,
            macAddress = connectionState.selectedDevice?.address,
            connectionState = connectionState.connectionState,
            isOperationInProgress = factoryState.isOperationInProgress,
            onDisconnect = { connectionViewModel.disconnect() },
            onNavigateHome = onNavigateHome
        )

        // Meter Type Card
        Card(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                Text(
                    text = "Meter Type",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(bottom = 16.dp)
                )

                Button(
                    onClick = { factoryViewModel.requestSetMeterType() },
                    modifier = Modifier.fillMaxWidth(),
                    enabled = !factoryState.isOperationInProgress,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.secondary
                    )
                ) {
                    Text("Imp-Exp/ABS/NET")
                }
            }
        }

        // Measured Data Card
        Card(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                Text(
                    text = "Measured Data",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(bottom = 16.dp)
                )

                Button(
                    onClick = { factoryViewModel.requestWholeClear() },
                    modifier = Modifier.fillMaxWidth(),
                    enabled = !factoryState.isOperationInProgress,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Text("Whole Clear")
                }

                Surface(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(top = 12.dp),
                    color = MaterialTheme.colorScheme.errorContainer,
                    shape = MaterialTheme.shapes.small
                ) {
                    Text(
                        text = "⚠️ Warning: This will clear ALL measured data",
                        modifier = Modifier.padding(8.dp),
                        color = MaterialTheme.colorScheme.onErrorContainer,
                        style = MaterialTheme.typography.bodySmall
                    )
                }
            }
        }

        // Missing Neutral Detection Card
        Card(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                Text(
                    text = "Missing Neutral Detection",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(bottom = 16.dp)
                )

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = { factoryViewModel.requestMissingNeutralOn() },
                        modifier = Modifier.weight(1f),
                        enabled = !factoryState.isOperationInProgress,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.tertiary
                        )
                    ) {
                        Text("MISN ON")
                    }

                    Button(
                        onClick = { factoryViewModel.requestMissingNeutralOff() },
                        modifier = Modifier.weight(1f),
                        enabled = !factoryState.isOperationInProgress,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.tertiary
                        )
                    ) {
                        Text("MISN OFF")
                    }
                }
            }
        }

        // Power Network Configuration Card
        Card(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                Text(
                    text = "Power Network Configuration",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(bottom = 16.dp)
                )

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = { factoryViewModel.requestPowerNetworkOne() },
                        modifier = Modifier.weight(1f),
                        enabled = !factoryState.isOperationInProgress
                    ) {
                        Text("PN ONE")
                    }

                    Button(
                        onClick = { factoryViewModel.requestPowerNetworkTwo() },
                        modifier = Modifier.weight(1f),
                        enabled = !factoryState.isOperationInProgress
                    ) {
                        Text("PN TWO")
                    }
                }

                Spacer(modifier = Modifier.height(8.dp))

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = { factoryViewModel.requestPotential() },
                        modifier = Modifier.weight(1f),
                        enabled = !factoryState.isOperationInProgress
                    ) {
                        Text("POTENT")
                    }

                    Button(
                        onClick = { factoryViewModel.requestTypeSet() },
                        modifier = Modifier.weight(1f),
                        enabled = !factoryState.isOperationInProgress
                    ) {
                        Text("TYPSEI")
                    }
                }
            }
        }

        // Operation Log Card
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .height(300.dp)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp)
            ) {
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

                    if (factoryState.isOperationInProgress) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(20.dp),
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
                    text = factoryState.lastResponse.ifEmpty { "No operations performed" },
                    style = MaterialTheme.typography.bodySmall,
                    modifier = Modifier
                        .fillMaxSize()
                        .verticalScroll(rememberScrollState())
                )
            }
        }
    }

    // Confirmation Dialog
    ConfirmationDialog(
        showDialog = factoryState.dialogState.show,
        title = factoryState.dialogState.title,
        message = factoryState.dialogState.message,
        confirmText = factoryState.dialogState.confirmText,
        isDestructive = factoryState.dialogState.isDestructive,
        onConfirm = factoryState.dialogState.onConfirm,
        onDismiss = { factoryViewModel.dismissDialog() }
    )
}