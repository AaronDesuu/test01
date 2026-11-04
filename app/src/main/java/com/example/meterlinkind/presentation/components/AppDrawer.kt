package com.example.meterlinkind.presentation.components

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.Logout
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

data class DrawerEntry(val route: String, val label: String, val icon: ImageVector)

/** The object screens, in menu order; the Meter screen's Browse tiles show the same list. */
val OBJECT_ENTRIES = listOf(
    DrawerEntry("data", "Data", Icons.Default.DataObject),
    DrawerEntry("register", "Register", Icons.Default.Numbers),
    DrawerEntry("extregister", "Extended Register", Icons.Default.AvTimer),
    DrawerEntry("profile", "Profile Generic", Icons.Default.TableChart),
    DrawerEntry("clock", "Clock", Icons.Default.Schedule),
    DrawerEntry("disconnect", "Disconnect Control", Icons.Default.PowerSettingsNew),
    DrawerEntry("limiter", "Limiter", Icons.Default.Tune),
)

private val METER_ENTRIES = listOf(DrawerEntry("home", "Meter", Icons.Default.Speed)) + OBJECT_ENTRIES

private val ACCOUNT_ENTRIES = listOf(
    DrawerEntry("settings", "Settings", Icons.Default.Settings),
)

@Composable
fun AppDrawer(
    currentRoute: String,
    navigate: (String) -> Unit,
    logout: () -> Unit,
    connect: () -> Unit,
    closeDrawer: () -> Unit,
    connectionViewModel: MeterConnectionViewModel
) {
    val connectionState by connectionViewModel.uiState.collectAsState()
    Box(modifier = Modifier.width(300.dp)) {
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

            METER_ENTRIES.forEach { DrawerItem(it, currentRoute, navigate, closeDrawer) }

            HorizontalDivider(modifier = Modifier.padding(horizontal = 28.dp, vertical = 8.dp))

            ACCOUNT_ENTRIES.forEach { DrawerItem(it, currentRoute, navigate, closeDrawer) }
            NavigationDrawerItem(
                label = { Text("Log Out") },
                icon = { Icon(Icons.AutoMirrored.Filled.Logout, contentDescription = null) },
                selected = false,
                onClick = {
                    closeDrawer()
                    logout()
                },
                modifier = Modifier.padding(NavigationDrawerItemDefaults.ItemPadding)
            )
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
            navigate(entry.route)
            closeDrawer()
        },
        modifier = Modifier.padding(NavigationDrawerItemDefaults.ItemPadding)
    )
}
