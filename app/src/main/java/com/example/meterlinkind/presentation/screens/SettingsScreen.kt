package com.example.meterlinkind.presentation.screens

import androidx.compose.material.icons.filled.School
import com.example.meterlinkind.presentation.components.RoleBadge
import androidx.compose.material.icons.filled.Person
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
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.KeyboardArrowRight
import androidx.compose.material.icons.automirrored.filled.Logout
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.ContentCopy
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.Key
import androidx.compose.material.icons.filled.Numbers
import androidx.compose.material.icons.filled.Storage
import androidx.compose.material.icons.filled.TableChart
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.model.NewMeterListMode
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
import com.example.meterlinkind.presentation.viewmodel.ObjectBrowserViewModel

/** Decimals choices: null keeps what the meter sent. */
private val DECIMAL_OPTIONS = listOf<Int?>(null, 2, 3)

/**
 * The logged-in client, then one row per setting showing its current value,
 * grouped as Security, Reading, Object lists and About, then Log out.
 */
@Composable
fun SettingsScreen(
    connectionViewModel: MeterConnectionViewModel,
    browserViewModel: ObjectBrowserViewModel,
    onOpenKeys: () -> Unit,
    onOpenSavedMeters: () -> Unit,
    onOpenTutorial: () -> Unit,
    modifier: Modifier = Modifier
) {
    val connection by connectionViewModel.uiState.collectAsState()
    val browser by browserViewModel.uiState.collectAsState()
    val savedCount = remember(browser.objectList) { browserViewModel.savedMeters().size }
    val context = LocalContext.current
    val version = remember {
        runCatching { context.packageManager.getPackageInfo(context.packageName, 0).versionName }.getOrNull() ?: "?"
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        AccountCard(connection.client)

        SectionTitle("Security")
        SettingsCard {
            NavRow(Icons.Default.Key, "Security keys", "Password, block cipher, authentication and broadcast keys", onOpenKeys)
        }

        SectionTitle("Reading")
        SettingsCard {
            ChoiceRow(
                icon = Icons.Default.TableChart,
                title = "Buffer reading starts at",
                selected = browser.bufferReadMode,
                options = BufferReadMode.entries,
                label = { it.label },
                onChange = browserViewModel::setBufferReadMode,
            )
            HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant)
            ChoiceRow(
                icon = Icons.Default.Numbers,
                title = "Decimals",
                selected = browser.decimals,
                options = DECIMAL_OPTIONS,
                label = ::decimalsLabel,
                onChange = browserViewModel::setDecimals,
            )
        }

        SectionTitle("Object lists")
        SettingsCard {
            ChoiceRow(
                icon = Icons.Default.ContentCopy,
                title = "New meters",
                selected = browser.newMeterListMode,
                options = NewMeterListMode.entries,
                label = { it.label },
                onChange = browserViewModel::setNewMeterListMode,
            )
            HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant)
            NavRow(
                Icons.Default.Storage,
                "Saved meters",
                when (savedCount) {
                    0 -> "No object lists saved"
                    1 -> "1 object list saved"
                    else -> "$savedCount object lists saved"
                },
                onOpenSavedMeters
            )
        }

        SectionTitle("About")
        SettingsCard {
            NavRow(Icons.Default.School, "Tutorial", "How to use Meter Link", onOpenTutorial)
            HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant)
            SettingRow(Icons.Default.Info, "Version", version, onClick = null) {}
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

/** "2 places · 4.0099998 → 4.01", or "As the meter sends them". */
private fun decimalsLabel(decimals: Int?): String = when (decimals) {
    null -> "As the meter sends them"
    else -> "$decimals places · 4.0099998 → ${"%.${decimals}f".format(4.0099998)}"
}

/** The logged-in client: name and its badge. */
@Composable
private fun AccountCard(client: ClientProfile?) {
    SettingsCard {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(14.dp)
        ) {
            RowIcon(Icons.Default.Person)
            Text(
                client?.label ?: "Not logged in",
                style = MaterialTheme.typography.titleSmall,
                modifier = Modifier.weight(1f)
            )
            client?.let { RoleBadge(it) }
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

/** Icon, name and current value; [trailing] at the end. [onClick] null makes it plain. */
@Composable
private fun SettingRow(
    icon: ImageVector,
    title: String,
    value: String,
    onClick: (() -> Unit)?,
    trailing: @Composable () -> Unit,
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .then(if (onClick != null) Modifier.clickable(onClick = onClick) else Modifier)
            .padding(12.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(14.dp)
    ) {
        RowIcon(icon)
        Column(modifier = Modifier.weight(1f)) {
            Text(title, style = MaterialTheme.typography.titleSmall)
            Text(value, style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }
        trailing()
    }
}

@Composable
private fun RowIcon(icon: ImageVector) {
    Surface(shape = RoundedCornerShape(10.dp), color = MaterialTheme.colorScheme.primaryContainer) {
        Icon(
            icon,
            contentDescription = null,
            tint = MaterialTheme.colorScheme.primary,
            modifier = Modifier
                .padding(8.dp)
                .size(20.dp)
        )
    }
}

@Composable
private fun NavRow(icon: ImageVector, title: String, value: String, onClick: () -> Unit) {
    SettingRow(icon, title, value, onClick) {
        Icon(Icons.AutoMirrored.Filled.KeyboardArrowRight, contentDescription = null)
    }
}

/** A setting whose current value shows under its name; a tap opens a menu of the choices. */
@Composable
private fun <T> ChoiceRow(
    icon: ImageVector,
    title: String,
    selected: T,
    options: List<T>,
    label: (T) -> String,
    onChange: (T) -> Unit,
) {
    var open by remember { mutableStateOf(false) }
    Box {
        SettingRow(icon, title, label(selected), onClick = { open = true }) {
            Icon(Icons.AutoMirrored.Filled.KeyboardArrowRight, contentDescription = null)
        }
        DropdownMenu(expanded = open, onDismissRequest = { open = false }) {
            options.forEach { option ->
                DropdownMenuItem(
                    text = { Text(label(option)) },
                    onClick = { onChange(option); open = false },
                    trailingIcon = if (option == selected) {
                        { Icon(Icons.Default.Check, contentDescription = "Selected") }
                    } else null
                )
            }
        }
    }
}
