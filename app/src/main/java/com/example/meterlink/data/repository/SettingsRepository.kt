package com.example.meterlink.data.repository

import android.content.Context
import android.content.SharedPreferences
import dagger.hilt.android.qualifiers.ApplicationContext
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class SettingsRepository @Inject constructor(
    @ApplicationContext context: Context
) {
    private val prefs: SharedPreferences =
        context.getSharedPreferences("meter_settings", Context.MODE_PRIVATE)

    fun saveSettings(
        account: String,
        password: String,
        address: String,
        rank: String,
        scan: String,
        tick: String,
        interval: String
    ) {
        prefs.edit().apply {
            putString("account", account)
            putString("password", password)
            putString("address", address)
            putString("rank", rank)
            putString("scan", scan)
            putString("tick", tick)
            putString("interval", interval)
            apply()
        }
    }

    fun getSettings(): Map<String, String> {
        return mapOf(
            "account" to (prefs.getString("account", "") ?: ""),
            "password" to (prefs.getString("password", "00000000000000000000000000000000") ?: "00000000000000000000000000000000"),
            "address" to (prefs.getString("address", "7f") ?: "7f"),
            "rank" to (prefs.getString("rank", "03") ?: "03"),
            "scan" to (prefs.getString("scan", "3000") ?: "3000"),
            "tick" to (prefs.getString("tick", "150") ?: "150"),
            "interval" to (prefs.getString("interval", "100") ?: "100")
        )
    }
}