package com.example.meterlinkind.data.repository

import android.content.Context
import android.content.SharedPreferences
import com.example.meterlinkind.data.model.ClientSecurity
import com.example.meterlinkind.data.model.UserSession
import com.example.meterlinkind.data.protocol.ClientProfile
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

    fun getClientSecurity(client: ClientProfile): ClientSecurity {
        val p = client.name.lowercase()
        return ClientSecurity(
            authPassword = stored("${p}_auth_password"),
            blockCipherKey = stored("${p}_block_cipher_key"),
            authenticationKey = stored("${p}_authentication_key"),
            broadcastKey = stored("${p}_broadcast_key"),
        )
    }

    /** A saved key as hex, accepting the older 16-character ASCII form. */
    private fun stored(key: String): String =
        prefs.getString(key, null)?.let(ClientSecurity::fromStored) ?: ClientSecurity.DEFAULT

    fun saveClientSecurity(client: ClientProfile, security: ClientSecurity) {
        val p = client.name.lowercase()
        prefs.edit().apply {
            putString("${p}_auth_password", security.authPassword)
            putString("${p}_block_cipher_key", security.blockCipherKey)
            putString("${p}_authentication_key", security.authenticationKey)
            putString("${p}_broadcast_key", security.broadcastKey)
            apply()
        }
    }

    /**
     * Our invocation counter, the second half of every GCM IV. One counter for
     * all clients: they share the system title and may share a key, so separate
     * counters would reuse an IV under the same key. Never repeats, also across
     * app restarts.
     */
    /** Decimal places floats are shown with; null = as the meter sent them. Default 2. */
    fun getDecimals(): Int? = prefs.getInt("display_decimals", 2).takeIf { it >= 0 }

    fun setDecimals(decimals: Int?) {
        prefs.edit().putInt("display_decimals", decimals ?: -1).apply()
    }

    /** Name of the [com.example.meterlinkind.data.model.NewMeterListMode] for meters without a saved list. */
    fun getNewMeterListMode(): String = prefs.getString("new_meter_list_mode", null) ?: "ASK"

    fun setNewMeterListMode(mode: String) {
        prefs.edit().putString("new_meter_list_mode", mode).apply()
    }

    /** Name of the mode the buffer reader opens with (`BufferReadMode`). */
    fun getBufferReadMode(): String = prefs.getString("buffer_read_mode", null) ?: "LAST_DAYS"

    fun setBufferReadMode(mode: String) {
        prefs.edit().putString("buffer_read_mode", mode).apply()
    }

    /** The saved login, or null if nobody is logged in (expiry is the caller's call). */
    fun getSession(): UserSession? {
        val client = prefs.getString("session_client", null)
            ?.let { name -> ClientProfile.entries.firstOrNull { it.name == name } } ?: return null
        return UserSession(client, prefs.getLong("session_last_active", 0L))
    }

    fun saveSession(session: UserSession) {
        prefs.edit()
            .putString("session_client", session.client.name)
            .putLong("session_last_active", session.lastActiveMillis)
            .apply()
    }

    fun clearSession() {
        prefs.edit().remove("session_client").remove("session_last_active").apply()
    }

    @Synchronized
    fun nextFrameCounter(): Long {
        val next = (prefs.getLong("frame_counter", 0L) + 1) and 0xFFFFFFFFL
        prefs.edit().putLong("frame_counter", next).apply()
        return next
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