package com.example.meterlink

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.hilt.lifecycle.viewmodel.compose.hiltViewModel
import com.example.meterlink.presentation.screens.MeterConnectionScreen
import com.example.meterlink.presentation.viewmodel.MeterConnectionViewModel
import com.example.meterlink.ui.theme.MeterLinkTheme
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            MeterLinkTheme {
                val viewModel: MeterConnectionViewModel = hiltViewModel()

                MeterConnectionScreen(
                    viewModel = viewModel,
                )
            }
        }
    }
}