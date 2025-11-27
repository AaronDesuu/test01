package com.example.meterlink.presentation.screens

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MeterOperationsScreen(
    onBack: () -> Unit,
    onGetStatus: () -> Unit,
    onGetConfig: () -> Unit,
    onGetEnergy: () -> Unit
) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Meter Operations") },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Text("←")
                    }
                }
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            OperationButton("Get Status", onClick = onGetStatus)
            OperationButton("Get Configuration", onClick = onGetConfig)
            OperationButton("Get Energy", onClick = onGetEnergy)
        }
    }
}

@Composable
fun OperationButton(text: String, onClick: () -> Unit) {
    Button(
        onClick = onClick,
        modifier = Modifier.fillMaxWidth()
    ) {
        Text(text)
    }
}