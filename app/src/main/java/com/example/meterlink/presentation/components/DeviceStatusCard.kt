package com.example.meterlink.presentation.components

import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.CheckCircle
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlink.data.repository.BleConnectionState

@Composable
fun DeviceStatusCard(
    deviceName: String?,
    serialNumber: String?,
    macAddress: String?,
    connectionState: BleConnectionState,
    isOperationInProgress: Boolean,
    onDisconnect: () -> Unit,
    onNavigateHome: () -> Unit,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.primaryContainer
        )
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp)
        ) {
            // Device Name and Serial Number
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = deviceName ?: "No Device",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold
                )
                if (serialNumber != null) {
                    Text(
                        text = "S/N: $serialNumber",
                        style = MaterialTheme.typography.bodySmall,
                        fontWeight = FontWeight.Medium
                    )
                }
            }

            // MAC Address
            if (macAddress != null) {
                Text(
                    text = "MAC: $macAddress",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onPrimaryContainer.copy(alpha = 0.7f),
                    modifier = Modifier.padding(top = 4.dp)
                )
            }

            Spacer(modifier = Modifier.height(12.dp))

            // Disconnect Button and Status
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                when (connectionState) {
                    is BleConnectionState.Connected -> {
                        Button(
                            onClick = onDisconnect,
                            modifier = Modifier.height(36.dp),
                            enabled = !isOperationInProgress,
                            colors = ButtonDefaults.buttonColors(
                                containerColor = MaterialTheme.colorScheme.error
                            ),
                            contentPadding = PaddingValues(horizontal = 16.dp, vertical = 4.dp)
                        ) {
                            Text("Disconnect", style = MaterialTheme.typography.labelSmall)
                        }
                    }
                    is BleConnectionState.Disconnected, is BleConnectionState.Error -> {
                        Button(
                            onClick = onNavigateHome,
                            modifier = Modifier.height(36.dp),
                            contentPadding = PaddingValues(horizontal = 16.dp, vertical = 4.dp)
                        ) {
                            Text("Connect", style = MaterialTheme.typography.labelSmall)
                        }
                    }
                    else -> {
                        Spacer(modifier = Modifier.width(100.dp))
                    }
                }

                // Status Indicator
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    when {
                        isOperationInProgress -> {
                            Text("Processing", style = MaterialTheme.typography.bodySmall)
                            CircularProgressIndicator(
                                modifier = Modifier.size(20.dp),
                                strokeWidth = 2.dp
                            )
                        }
                        connectionState is BleConnectionState.Connected -> {
                            Text("Ready", style = MaterialTheme.typography.bodySmall, color = Color.Green)
                            Icon(
                                imageVector = Icons.Default.CheckCircle,
                                contentDescription = "Ready",
                                tint = Color.Green,
                                modifier = Modifier.size(20.dp)
                            )
                        }
                        connectionState is BleConnectionState.Connecting -> {
                            Text("Connecting", style = MaterialTheme.typography.bodySmall)
                            CircularProgressIndicator(
                                modifier = Modifier.size(20.dp),
                                strokeWidth = 2.dp
                            )
                        }
                        else -> {
                            Text(
                                "Disconnected",
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.error
                            )
                        }
                    }
                }
            }
        }
    }
}