package com.example.meterlink.presentation.components

import android.annotation.SuppressLint
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material3.DividerDefaults
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalDrawerSheet
import androidx.compose.material3.NavigationDrawerItem
import androidx.compose.material3.NavigationDrawerItemDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel

@SuppressLint("MissingPermission")
@Composable
fun AppDrawer(
    currentRoute: String,
    navigateToHome: () -> Unit,
    navigateToOperations: () -> Unit,
    navigateToMaintenance: () -> Unit,
    navigateToFactory: () -> Unit,
    navigateToSettings: () -> Unit,
    closeDrawer: () -> Unit,
    connectionViewModel: MeterConnectionViewModel
) {
    val connectionState by connectionViewModel.uiState.collectAsState()
    Box(modifier = Modifier.width(280.dp)) {
        ModalDrawerSheet {
            // Drawer Header
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                Text(
                    text = "Meter Link",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold
                )
                Text(
                    text = "E-Meter BLE Communication",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }

            HorizontalDivider(Modifier, DividerDefaults.Thickness, DividerDefaults.color)
            Spacer(Modifier.height(12.dp))

            // Device Status Card
            Column(modifier = Modifier.padding(horizontal = 12.dp, vertical = 8.dp)) {
                CompactDeviceStatusCard(
                    deviceName = connectionState.selectedDevice?.name,
                    serialNumber = connectionState.serialNumber,
                    macAddress = connectionState.selectedDevice?.address,
                    connectionState = connectionState.connectionState,
                    isOperationInProgress = connectionState.isOperationInProgress
                )
            }

            HorizontalDivider(Modifier, DividerDefaults.Thickness, DividerDefaults.color)
            Spacer(Modifier.height(8.dp))

            NavigationDrawerItem(
                label = { Text("Home") },
                selected = currentRoute == "home",
                onClick = {
                    navigateToHome()
                    closeDrawer()
                },
                modifier = Modifier.padding(NavigationDrawerItemDefaults.ItemPadding)
            )

            NavigationDrawerItem(
                label = { Text("Operations") },
                selected = currentRoute == "operations",
                onClick = {
                    navigateToOperations()
                    closeDrawer()
                },
                modifier = Modifier.padding(NavigationDrawerItemDefaults.ItemPadding)
            )

            NavigationDrawerItem(
                label = { Text("Maintenance") },
                selected = currentRoute == "maintenance",
                onClick = {
                    navigateToMaintenance()
                    closeDrawer()
                },
                modifier = Modifier.padding(NavigationDrawerItemDefaults.ItemPadding)
            )

            NavigationDrawerItem(
                label = { Text("Factory Setting") },
                selected = currentRoute == "factory",
                onClick = {
                    navigateToFactory()
                    closeDrawer()
                },
                modifier = Modifier.padding(NavigationDrawerItemDefaults.ItemPadding)
            )

            NavigationDrawerItem(
                label = { Text("Settings") },
                selected = currentRoute == "settings",
                onClick = {
                    navigateToSettings()
                    closeDrawer()
                },
                modifier = Modifier.padding(NavigationDrawerItemDefaults.ItemPadding)
            )
        }
    }
}