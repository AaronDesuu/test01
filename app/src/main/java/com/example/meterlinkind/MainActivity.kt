package com.example.meterlinkind

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.runtime.DisposableEffect
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
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

                // The 3-day session timeout counts from when the app was last in the foreground.
                val lifecycleOwner = LocalLifecycleOwner.current
                DisposableEffect(lifecycleOwner) {
                    val observer = LifecycleEventObserver { _, event ->
                        when (event) {
                            Lifecycle.Event.ON_RESUME -> viewModel.onAppResumed()
                            Lifecycle.Event.ON_PAUSE -> viewModel.onAppPaused()
                            else -> Unit
                        }
                    }
                    lifecycleOwner.lifecycle.addObserver(observer)
                    onDispose { lifecycleOwner.lifecycle.removeObserver(observer) }
                }

                MeterConnectionScreen(
                    viewModel = viewModel,
                )
            }
        }
    }
}