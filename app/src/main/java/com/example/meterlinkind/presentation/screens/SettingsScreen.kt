package com.example.meterlinkind.presentation.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.KeyboardArrowRight
import androidx.compose.material.icons.automirrored.filled.Logout
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.ButtonDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.presentation.components.RoleBadge
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
import com.example.meterlinkind.presentation.viewmodel.ObjectBrowserViewModel

/** Decimals choices: null keeps what the meter sent. */
private val DECIMAL_OPTIONS = listOf<Int?>(null, 2, 3)

/** Two sections: Account & security, and Data & display. */
@Composable
fun SettingsScreen(
    connectionViewModel: MeterConnectionViewModel,
    browserViewModel: ObjectBrowserViewModel,
    onOpenKeys: () -> Unit,
    onOpenSavedMeters: () -> Unit,
    modifier: Modifier = Modifier
) {
    val connection by connectionViewModel.uiState.collectAsState()
    val browser by browserViewModel.uiState.collectAsState()
    val savedCount = remember(browser.objectList) { browserViewModel.savedMeters().size }

    Column(
        modifier = modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        SectionTitle("Account & security")
        SettingsCard {
            Row(
                modifier = Modifier.padding(12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Column(modifier = Modifier.weight(1f)) {
                    Text(connection.client?.label ?: "Not logged in", style = MaterialTheme.typography.titleSmall)
                    Text(
                        "Logs out after 3 days without using the app",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                connection.client?.let { RoleBadge(it) }
            }
            HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant)
            NavRow("Security keys", "Password, block cipher, authentication and broadcast keys", onOpenKeys)
        }

        SectionTitle("Data & display")
        SettingsCard {
            ReadModeRow(browser.bufferReadMode, browserViewModel::setBufferReadMode)
            HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant)
            Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text("Decimals", style = MaterialTheme.typography.titleSmall)
                Text(
                    "Rounds measured values: 4.0099998 shows as 4.01 at 2.",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                    DECIMAL_OPTIONS.forEachIndexed { i, option ->
                        SegmentedButton(
                            selected = browser.decimals == option,
                            onClick = { browserViewModel.setDecimals(option) },
                            shape = SegmentedButtonDefaults.itemShape(i, DECIMAL_OPTIONS.size),
                            label = { Text(option?.toString() ?: "As sent") }
                        )
                    }
                }
            }
            HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant)
            NavRow(
                "Saved meters",
                when (savedCount) {
                    0 -> "No object lists saved"
                    1 -> "1 object list saved"
                    else -> "$savedCount object lists saved"
                },
                onOpenSavedMeters
            )
        }

        OutlinedButton(
            onClick = connectionViewModel::logout,
            modifier = Modifier
                .fillMaxWidth()
                .padding(top = 16.dp),
            colors = ButtonDefaults.outlinedButtonColors(contentColor = MaterialTheme.colorScheme.error)
        ) {
            Icon(Icons.AutoMirrored.Filled.Logout, contentDescription = null, modifier = Modifier.size(18.dp))
            Text("  Log out")
        }
    }
}

@Composable
private fun SectionTitle(text: String) {
    Text(
        text,
        style = MaterialTheme.typography.labelLarge,
        color = MaterialTheme.colorScheme.primary,
        modifier = Modifier.padding(start = 4.dp, top = 8.dp)
    )
}

@Composable
private fun SettingsCard(content: @Composable ColumnScope.() -> Unit) {
    Surface(shape = RoundedCornerShape(16.dp), color = MaterialTheme.colorScheme.surfaceContainerLow) {
        Column(content = content)
    }
}

@Composable
private fun NavRow(title: String, subtitle: String, onClick: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(12.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(title, style = MaterialTheme.typography.titleSmall)
            Text(subtitle, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }
        Icon(Icons.AutoMirrored.Filled.KeyboardArrowRight, contentDescription = null)
    }
}

/** The mode the buffer reader opens with, picked from a menu. */
@Composable
private fun ReadModeRow(mode: BufferReadMode, onChange: (BufferReadMode) -> Unit) {
    var open by remember { mutableStateOf(false) }
    Box {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .clickable { open = true }
                .padding(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text("Buffer reading starts at", style = MaterialTheme.typography.titleSmall)
                Text(
                    "The option the Reading box opens with",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            Text(
                mode.label,
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.Medium,
                color = MaterialTheme.colorScheme.onSecondaryContainer,
                modifier = Modifier
                    .background(MaterialTheme.colorScheme.secondaryContainer, CircleShape)
                    .padding(horizontal = 10.dp, vertical = 4.dp)
            )
        }
        DropdownMenu(expanded = open, onDismissRequest = { open = false }) {
            BufferReadMode.entries.forEach {
                DropdownMenuItem(text = { Text(it.label) }, onClick = { onChange(it); open = false })
            }
        }
    }
}
