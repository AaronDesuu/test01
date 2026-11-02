package com.example.meterlinkind.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

private val LightColorScheme = lightColorScheme(
    primary = Blue600,
    onPrimary = Color.White,
    primaryContainer = Blue100,
    onPrimaryContainer = Blue900,
    inversePrimary = Blue200,
    secondary = Slate500,
    onSecondary = Color.White,
    secondaryContainer = Slate100,
    onSecondaryContainer = Slate900,
    tertiary = Amber400,
    onTertiary = Amber950,
    tertiaryContainer = Amber100,
    onTertiaryContainer = Amber950,
    background = NeutralLight,
    onBackground = OnNeutralLight,
    surface = NeutralLight,
    onSurface = OnNeutralLight,
    surfaceVariant = NeutralVariantLight,
    onSurfaceVariant = OnNeutralVariantLight,
    surfaceTint = Blue600,
    surfaceContainerLowest = Color(0xFFFFFFFF),
    surfaceContainerLow = Color(0xFFF3F4FA),
    surfaceContainer = Color(0xFFEDEFF6),
    surfaceContainerHigh = Color(0xFFE7E9F1),
    surfaceContainerHighest = Color(0xFFE1E4EC),
    outline = OutlineLight,
    outlineVariant = OutlineVariantLight,
    error = ErrorLight,
    onError = Color.White,
    errorContainer = ErrorContainerLight,
    onErrorContainer = OnErrorContainerLight,
)

private val DarkColorScheme = darkColorScheme(
    primary = Blue200,
    onPrimary = Blue900,
    primaryContainer = Blue700,
    onPrimaryContainer = Blue100,
    inversePrimary = Blue600,
    secondary = Slate200,
    onSecondary = Slate900,
    secondaryContainer = Slate700,
    onSecondaryContainer = Slate100,
    tertiary = Amber300,
    onTertiary = Amber950,
    tertiaryContainer = Amber800,
    onTertiaryContainer = Amber100,
    background = NeutralDark,
    onBackground = OnNeutralDark,
    surface = NeutralDark,
    onSurface = OnNeutralDark,
    surfaceVariant = NeutralVariantDark,
    onSurfaceVariant = OnNeutralVariantDark,
    surfaceTint = Blue200,
    surfaceContainerLowest = Color(0xFF070912),
    surfaceContainerLow = Color(0xFF131726),
    surfaceContainer = Color(0xFF171B2A),
    surfaceContainerHigh = Color(0xFF212536),
    surfaceContainerHighest = Color(0xFF2C3041),
    outline = OutlineDark,
    outlineVariant = OutlineVariantDark,
    error = ErrorDark,
    onError = OnErrorDark,
    errorContainer = ErrorContainerDark,
    onErrorContainer = ErrorContainerLight,
)

/**
 * Always the brand scheme: Android 12+ dynamic (wallpaper) colour is not used,
 * so the app matches its icon on every phone.
 */
@Composable
fun MeterLinkINDTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit
) {
    MaterialTheme(
        colorScheme = if (darkTheme) DarkColorScheme else LightColorScheme,
        typography = Typography,
        content = content
    )
}
