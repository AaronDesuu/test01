package com.example.meterlinkind.presentation.components

import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.filled.Alarm
import androidx.compose.material.icons.filled.Cable
import androidx.compose.material.icons.filled.CloudUpload
import androidx.compose.material.icons.filled.Code
import androidx.compose.material.icons.filled.Event
import androidx.compose.material.icons.filled.Groups
import androidx.compose.material.icons.filled.Lan
import androidx.compose.material.icons.filled.Public
import androidx.compose.material.icons.filled.Shield
import androidx.compose.material.icons.filled.SignalCellularAlt
import androidx.compose.material.icons.filled.SystemUpdate
import androidx.compose.ui.draw.alpha
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AvTimer
import androidx.compose.material.icons.filled.DataObject
import androidx.compose.material.icons.filled.Numbers
import androidx.compose.material.icons.filled.PowerSettingsNew
import androidx.compose.material.icons.filled.Schedule
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material.icons.filled.Speed
import androidx.compose.material.icons.filled.TableChart
import androidx.compose.material.icons.filled.Tune
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalDrawerSheet
import androidx.compose.material3.NavigationDrawerItem
import androidx.compose.material3.NavigationDrawerItemDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel

/** A menu entry; a disabled one shows greyed out and can't be opened. */
data class DrawerEntry(val route: String, val label: String, val icon: ImageVector, val enabled: Boolean = true)

/** The object screens in Gurux's order (by interface class); the Meter screen's tiles show the same list. */
val OBJECT_ENTRIES = listOf(
    DrawerEntry("data", "Data", Icons.Default.DataObject),
    DrawerEntry("register", "Register", Icons.Default.Numbers),
    DrawerEntry("extregister", "Extended Register", Icons.Default.AvTimer),
    DrawerEntry("profile", "Profile Generic", Icons.Default.TableChart),
    DrawerEntry("clock", "Clock", Icons.Default.Schedule),
    DrawerEntry("script", "Script Table", Icons.Default.Code),
    DrawerEntry("specialdays", "Special Days Table", Icons.Default.Event),
    DrawerEntry("association", "Association LN", Icons.Default.Groups),
    // Firmware upgrade isn't supported by the app yet.
    DrawerEntry("imagetransfer", "Image Transfer", Icons.Default.SystemUpdate, enabled = false),
    DrawerEntry("actionschedule", "Action Schedule", Icons.Default.Alarm),
    DrawerEntry("hdlc", "IEC HDLC Setup", Icons.Default.Cable),
    DrawerEntry("push", "Push Setup", Icons.Default.CloudUpload),
    DrawerEntry("tcpudp", "TCP-UDP Setup", Icons.Default.Lan),
    DrawerEntry("gprs", "GPRS Setup", Icons.Default.SignalCellularAlt),
    DrawerEntry("ipv6", "IPv6 Setup", Icons.Default.Public),
    DrawerEntry("security", "Security Setup", Icons.Default.Shield),
    DrawerEntry("disconnect", "Disconnect Control", Icons.Default.PowerSettingsNew),
    DrawerEntry("limiter", "Limiter", Icons.Default.Tune),
)

private val METER_ENTRY = DrawerEntry("home", "Meter", Icons.Default.Speed)

private val ACCOUNT_ENTRIES = listOf(
    DrawerEntry("settings", "Settings", Icons.Default.Settings),
)

@Composable
fun AppDrawer(
    currentRoute: String,
    navigate: (String) -> Unit,
    connect: () -> Unit,
    closeDrawer: () -> Unit,
    connectionViewModel: MeterConnectionViewModel
) {
    val connectionState by connectionViewModel.uiState.collectAsState()
    Box(modifier = Modifier.width(300.dp)) {
        // The header, the meter card, Meter and Settings stay put; only the object entries scroll.
        ModalDrawerSheet {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        text = "Meter Link",
                        style = MaterialTheme.typography.titleLarge,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.weight(1f)
                    )
                    connectionState.client?.let { RoleBadge(it) }
                }
                connectionState.client?.let {
                    Text(
                        text = "Logged in as ${it.label}",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                Spacer(Modifier.height(12.dp))
                ConnectionStrip(
                    deviceName = connectionState.selectedDeviceName,
                    serialNumber = connectionState.serialNumber,
                    connectionState = connectionState.connectionState,
                    associated = connectionState.isDlmsConnected,
                    onConnect = {
                        closeDrawer()
                        connect()
                    },
                )
            }

            DrawerItem(METER_ENTRY, currentRoute, navigate, closeDrawer)
            HorizontalDivider(modifier = Modifier.padding(horizontal = 28.dp, vertical = 4.dp))

            Column(
                modifier = Modifier
                    .weight(1f)
                    .verticalScroll(rememberScrollState())
            ) {
                OBJECT_ENTRIES.forEach { DrawerItem(it, currentRoute, navigate, closeDrawer) }
            }

            HorizontalDivider(modifier = Modifier.padding(horizontal = 28.dp, vertical = 4.dp))
            ACCOUNT_ENTRIES.forEach { DrawerItem(it, currentRoute, navigate, closeDrawer) }
            Spacer(Modifier.height(8.dp))
        }
    }
}

@Composable
private fun DrawerItem(entry: DrawerEntry, currentRoute: String, navigate: (String) -> Unit, closeDrawer: () -> Unit) {
    NavigationDrawerItem(
        label = { Text(entry.label) },
        icon = { Icon(entry.icon, contentDescription = null) },
        selected = currentRoute == entry.route,
        onClick = {
            if (entry.enabled) {
                navigate(entry.route)
                closeDrawer()
            }
        },
        badge = if (entry.enabled) null else {
            { Text("Off", style = MaterialTheme.typography.labelSmall) }
        },
        modifier = Modifier
            .padding(NavigationDrawerItemDefaults.ItemPadding)
            .alpha(if (entry.enabled) 1f else DISABLED_ALPHA)
    )
}

/** Opacity of entries that can't be opened. */
const val DISABLED_ALPHA = 0.38f
