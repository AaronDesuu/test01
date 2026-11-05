package com.example.meterlinkind.presentation.components

import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.border
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
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.data.repository.BleConnectionState

/** Status colours, the same in both themes. */
internal val OnlineColor = Color(0xFF1E8E5A)
private val OnlineContainer = Color(0xFFD5F3E3)

/**
 * The drawer's connection line: device, serial, client and link state. While
 * offline, tapping it calls [onConnect] (reconnect to the last meter, or scan).
 */
@Composable
fun ConnectionStrip(
    deviceName: String?,
    serialNumber: String?,
    connectionState: BleConnectionState,
    associated: Boolean,
    onConnect: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val online = connectionState is BleConnectionState.Connected && associated
    val connecting = connectionState is BleConnectionState.Connecting ||
        (connectionState is BleConnectionState.Connected && !associated)
    Surface(
        onClick = onConnect,
        enabled = !online && !connecting,
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
                    when {
                        online -> serialNumber?.let { "S/N $it" } ?: "Reading serial..."
                        connecting -> "Connecting..."
                        else -> "Click here to connect"
                    },
                    style = MaterialTheme.typography.bodySmall,
                    fontFamily = if (online) FontFamily.Monospace else null
                )
            }
            StatusChip(connectionState, associated)
        }
    }
}

/**
 * Meter status in the top bar of the object screens: a green dot and the
 * meter's name when online, a blinking blue dot and "Connecting", or a hollow
 * grey dot and "Offline".
 */
@Composable
fun MeterStatusPill(
    deviceName: String?,
    connectionState: BleConnectionState,
    associated: Boolean,
    onClick: () -> Unit,
) {
    val online = connectionState is BleConnectionState.Connected && associated
    val connecting = connectionState is BleConnectionState.Connecting ||
        (connectionState is BleConnectionState.Connected && !associated)
    val (container, content) = when {
        online -> OnlineContainer to OnlineColor
        connecting -> MaterialTheme.colorScheme.primaryContainer to MaterialTheme.colorScheme.primary
        else -> MaterialTheme.colorScheme.surfaceContainerHigh to MaterialTheme.colorScheme.onSurfaceVariant
    }
    val blink by rememberInfiniteTransition(label = "pill dot").animateFloat(
        initialValue = 1f,
        targetValue = 0.25f,
        animationSpec = infiniteRepeatable(tween(500), RepeatMode.Reverse),
        label = "pill dot alpha"
    )
    Surface(onClick = onClick, shape = CircleShape, color = container, modifier = Modifier.padding(end = 8.dp)) {
        Row(
            modifier = Modifier.padding(start = 8.dp, end = 10.dp, top = 5.dp, bottom = 5.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(6.dp)
        ) {
            val dot = Modifier.size(8.dp)
            when {
                online -> Box(dot.background(content, CircleShape))
                connecting -> Box(dot.graphicsLayer { alpha = blink }.background(content, CircleShape))
                else -> Box(dot.border(1.5.dp, content, CircleShape))
            }
            Text(
                text = when {
                    online -> deviceName ?: "Meter"
                    connecting -> "Connecting"
                    else -> "Offline"
                },
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Medium,
                color = if (online) MaterialTheme.colorScheme.onSurface else content,
                maxLines = 1
            )
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

/** Badge for the logged-in DLMS client. */
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
