package com.example.meterlink.presentation.components

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Divider
import androidx.compose.material3.DividerDefaults
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalDrawerSheet
import androidx.compose.material3.NavigationDrawerItem
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
fun AppDrawer(
    currentRoute: String,
    navigateToHome: () -> Unit,
    navigateToOperations: () -> Unit,
    navigateToSettings: () -> Unit,
    closeDrawer: () -> Unit
) {
    ModalDrawerSheet {
        Text(
            "MeterLink",
            modifier = Modifier.padding(16.dp),
            style = MaterialTheme.typography.titleLarge
        )
        HorizontalDivider(Modifier, DividerDefaults.Thickness, DividerDefaults.color)

        NavigationDrawerItem(
            label = { Text("Home") },
            selected = currentRoute == "home",
            onClick = {
                navigateToHome()
                closeDrawer()
            }
        )
        NavigationDrawerItem(
            label = { Text("Operations") },
            selected = currentRoute == "operations",
            onClick = {
                navigateToOperations()
                closeDrawer()
            }
        )
        NavigationDrawerItem(
            label = { Text("Settings") },
            selected = currentRoute == "settings",
            onClick = {
                navigateToSettings()
                closeDrawer()
            }
        )
    }
}