package com.example.meterlinkind

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.hilt.lifecycle.viewmodel.compose.hiltViewModel
import com.example.meterlinkind.presentation.screens.MeterConnectionScreen
import com.example.meterlinkind.presentation.viewmodel.MeterConnectionViewModel
import com.example.meterlinkind.ui.theme.MeterLinkINDTheme
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            MeterLinkINDTheme {
                val viewModel: MeterConnectionViewModel = hiltViewModel()

                MeterConnectionScreen(
                    viewModel = viewModel,
                )
            }
        }
    }
}