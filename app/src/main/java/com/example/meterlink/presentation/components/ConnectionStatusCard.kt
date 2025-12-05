package com.example.meterlink.presentation.components

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.meterlink.data.repository.BleConnectionState

@Composable
fun ConnectionStatusCard(
    state: BleConnectionState,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = when (state) {
                is BleConnectionState.Connected -> MaterialTheme.colorScheme.primaryContainer
                is BleConnectionState.Error -> MaterialTheme.colorScheme.errorContainer
                else -> MaterialTheme.colorScheme.surfaceVariant
            }
        )
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text(
                text = when (state) {
                    is BleConnectionState.Disconnected -> "Disconnected"
                    is BleConnectionState.Connecting -> "Connecting..."
                    is BleConnectionState.Connected -> "Connected"
                    is BleConnectionState.Error -> "Error"
                },
                style = MaterialTheme.typography.headlineSmall
            )

            if (state is BleConnectionState.Error) {
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = state.message,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onErrorContainer
                )
            }
        }
    }
}