package com.example.meterlinkind.presentation.components

import androidx.compose.animation.AnimatedContent
import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.LinearOutSlowInEasing
import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.Spring
import androidx.compose.animation.core.StartOffset
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.keyframes
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.spring
import androidx.compose.animation.core.tween
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.scaleIn
import androidx.compose.animation.slideInVertically
import androidx.compose.animation.slideOutVertically
import androidx.compose.animation.togetherWith
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Bluetooth
import androidx.compose.material.icons.filled.BluetoothDisabled
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.ErrorOutline
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.data.repository.BleConnectionState

/**
 * The meter card's status circle (motion option B): grey when idle, rings
 * rippling out while connecting, a check that pops in when connected, red on
 * failure. Shrinks a little when connected to make room for the details.
 */
@Composable
fun MeterStatusBadge(state: BleConnectionState, modifier: Modifier = Modifier) {
    val colors = MaterialTheme.colorScheme
    val background by animateColorAsState(
        when (state) {
            is BleConnectionState.Connected -> colors.primary
            is BleConnectionState.Connecting -> colors.primaryContainer
            is BleConnectionState.Error -> colors.errorContainer
            else -> colors.surfaceVariant
        },
        tween(250), label = "badge background"
    )
    val scale by animateFloatAsState(
        if (state is BleConnectionState.Connected) 0.82f else 1f,
        tween(320), label = "badge scale"
    )

    Box(modifier = modifier.size(80.dp).graphicsLayer { scaleX = scale; scaleY = scale }, contentAlignment = Alignment.Center) {
        if (state is BleConnectionState.Connecting) ConnectingRings(colors.primary)
        Box(Modifier.size(80.dp).background(background, CircleShape), contentAlignment = Alignment.Center) {
            AnimatedContent(
                targetState = state.iconKind(),
                transitionSpec = {
                    // The check pops in with a bounce; the other icons just fade.
                    val enter = if (targetState == IconKind.CONNECTED) {
                        scaleIn(spring(Spring.DampingRatioMediumBouncy, Spring.StiffnessMedium))
                    } else {
                        fadeIn(tween(180))
                    }
                    enter togetherWith fadeOut(tween(120))
                },
                label = "badge icon"
            ) { kind ->
                Icon(
                    imageVector = when (kind) {
                        IconKind.CONNECTED -> Icons.Default.Check
                        IconKind.CONNECTING -> Icons.Default.Bluetooth
                        IconKind.FAILED -> Icons.Default.ErrorOutline
                        IconKind.IDLE -> Icons.Default.BluetoothDisabled
                    },
                    contentDescription = null,
                    modifier = Modifier.size(40.dp),
                    tint = when (kind) {
                        IconKind.CONNECTED -> colors.onPrimary
                        IconKind.CONNECTING -> colors.primary
                        IconKind.FAILED -> colors.error
                        IconKind.IDLE -> colors.onSurfaceVariant
                    }
                )
            }
        }
    }
}

private enum class IconKind { IDLE, CONNECTING, CONNECTED, FAILED }

private fun BleConnectionState.iconKind(): IconKind = when (this) {
    is BleConnectionState.Connected -> IconKind.CONNECTED
    is BleConnectionState.Connecting -> IconKind.CONNECTING
    is BleConnectionState.Error -> IconKind.FAILED
    else -> IconKind.IDLE
}

/** Two rings growing out of the circle and fading, half a cycle apart. */
@Composable
private fun ConnectingRings(color: Color) {
    val transition = rememberInfiniteTransition(label = "connecting rings")
    repeat(2) { i ->
        val progress by transition.animateFloat(
            initialValue = 0f,
            targetValue = 1f,
            animationSpec = infiniteRepeatable(
                tween(1400, easing = LinearOutSlowInEasing),
                RepeatMode.Restart,
                StartOffset(i * 700)
            ),
            label = "ring $i"
        )
        Box(
            Modifier
                .size(80.dp)
                .graphicsLayer {
                    scaleX = 1f + 0.75f * progress
                    scaleY = 1f + 0.75f * progress
                    alpha = 0.55f * (1f - progress)
                }
                .border(2.dp, color, CircleShape)
        )
    }
}

/**
 * The status line (motion option D): the old text slides up and out while the
 * new one slides in from below.
 */
@Composable
fun MeterStatusTitle(text: String) {
    AnimatedContent(
        targetState = text,
        transitionSpec = {
            (slideInVertically(tween(320)) { it } + fadeIn(tween(320))) togetherWith
                (slideOutVertically(tween(320)) { -it } + fadeOut(tween(320)))
        },
        label = "status title"
    ) { title ->
        Text(title, style = MaterialTheme.typography.headlineSmall, fontWeight = FontWeight.Bold)
    }
}

/**
 * Horizontal offset in px for one short shake each time [failed] turns true
 * (a connection attempt just failed). Apply it with Modifier.offset.
 */
@Composable
fun rememberFailureShake(failed: Boolean): Animatable<Float, *> {
    val offset = remember { Animatable(0f) }
    LaunchedEffect(failed) {
        if (failed) {
            offset.animateTo(0f, keyframes {
                durationMillis = 380
                -24f at 76
                21f at 152
                -15f at 228
                9f at 304
            })
        }
    }
    return offset
}
