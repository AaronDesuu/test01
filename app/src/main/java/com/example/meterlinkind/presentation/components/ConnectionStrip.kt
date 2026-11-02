package com.example.meterlinkind.presentation.components

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Bluetooth
import androidx.compose.material.icons.filled.BluetoothDisabled
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.data.repository.BleConnectionState

/** Status colours: meaning, not brand, so they stay green/red in both themes. */
private val OnlineColor = Color(0xFF1E8E5A)
private val OnlineContainer = Color(0xFFD5F3E3)

/**
 * One line naming the meter this screen talks to: device, serial, client and
 * link state. Sits at the top of every meter screen.
 */
@Composable
fun ConnectionStrip(
    deviceName: String?,
    serialNumber: String?,
    connectionState: BleConnectionState,
    associated: Boolean,
    modifier: Modifier = Modifier,
) {
    val online = connectionState is BleConnectionState.Connected && associated
    Surface(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(14.dp),
        color = if (online) MaterialTheme.colorScheme.primaryContainer else MaterialTheme.colorScheme.surfaceContainerHigh,
        contentColor = if (online) MaterialTheme.colorScheme.onPrimaryContainer else MaterialTheme.colorScheme.onSurfaceVariant,
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 12.dp, vertical = 10.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            Icon(
                if (online) Icons.Default.Bluetooth else Icons.Default.BluetoothDisabled,
                contentDescription = null,
                modifier = Modifier.size(20.dp)
            )
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    deviceName ?: "No meter",
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.SemiBold
                )
                Text(
                    serialNumber?.let { "S/N $it" } ?: if (online) "Reading serial..." else "Connect from Home",
                    style = MaterialTheme.typography.bodySmall,
                    fontFamily = FontFamily.Monospace
                )
            }
            StatusChip(connectionState, associated)
        }
    }
}

@Composable
fun StatusChip(connectionState: BleConnectionState, associated: Boolean) {
    val (label, fg, bg) = when {
        connectionState is BleConnectionState.Connected && associated ->
            Triple("Online", OnlineColor, OnlineContainer)
        connectionState is BleConnectionState.Connecting || connectionState is BleConnectionState.Connected ->
            Triple("Connecting", MaterialTheme.colorScheme.onSecondaryContainer, MaterialTheme.colorScheme.secondaryContainer)
        else -> Triple("Offline", MaterialTheme.colorScheme.onSurfaceVariant, MaterialTheme.colorScheme.surfaceVariant)
    }
    Row(
        modifier = Modifier
            .background(bg, CircleShape)
            .padding(horizontal = 8.dp, vertical = 3.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(5.dp)
    ) {
        Box(Modifier.size(6.dp).background(fg, CircleShape))
        Text(label, style = MaterialTheme.typography.labelSmall, color = fg, fontWeight = FontWeight.Medium)
    }
}

/** The logged-in DLMS client, amber like the icon's needle. */
@Composable
fun RoleBadge(client: ClientProfile) {
    Text(
        text = when (client) {
            ClientProfile.UTILITY_SETTINGS -> "US"
            ClientProfile.METER_READER -> "MR"
            ClientProfile.PUBLIC -> "PC"
        },
        style = MaterialTheme.typography.labelMedium,
        fontWeight = FontWeight.Bold,
        color = MaterialTheme.colorScheme.onTertiaryContainer,
        modifier = Modifier
            .background(MaterialTheme.colorScheme.tertiaryContainer, CircleShape)
            .padding(horizontal = 10.dp, vertical = 3.dp)
    )
}
