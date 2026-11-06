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

    /**
     * The meter passwords and keys, encrypted by [vault]. A file of its own so
     * the backup rules can leave it out: the Keystore key never leaves this
     * phone, so a restored copy couldn't be opened anyway.
     */
    private val keys: SharedPreferences =
        context.getSharedPreferences("meter_keys", Context.MODE_PRIVATE)

    private val vault = KeyVault()

    init {
        // Encrypt every client's keys left in plain text by older builds, not
        // just those of whoever logs in next.
        ClientProfile.entries.forEach(::getClientSecurity)
    }

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

    /**
     * A saved key as hex. Older builds kept the keys in plain text in
     * `meter_settings`; such a value is encrypted into `meter_keys` and removed
     * the first time it is read.
     */
    private fun stored(key: String): String {
        keys.getString(key, null)?.let { sealed ->
            return vault.open(key, sealed)?.let(ClientSecurity::fromStored) ?: ClientSecurity.DEFAULT
        }
        val plain = prefs.getString(key, null)?.let(ClientSecurity::fromStored) ?: return ClientSecurity.DEFAULT
        keys.edit().putString(key, vault.seal(key, plain)).apply()
        prefs.edit().remove(key).apply()
        return plain
    }

    fun saveClientSecurity(client: ClientProfile, security: ClientSecurity) {
        val p = client.name.lowercase()
        val values = mapOf(
            "${p}_auth_password" to security.authPassword,
            "${p}_block_cipher_key" to security.blockCipherKey,
            "${p}_authentication_key" to security.authenticationKey,
            "${p}_broadcast_key" to security.broadcastKey,
        )
        keys.edit().apply {
            values.forEach { (name, value) -> putString(name, vault.seal(name, value)) }
            apply()
        }
        prefs.edit().apply {
            values.keys.forEach(::remove)
            apply()
        }
    }

    /** True once the tutorial was finished or skipped; it then no longer opens by itself. */
    fun isTutorialSeen(): Boolean = prefs.getBoolean("tutorial_seen", false)

    fun setTutorialSeen() {
        prefs.edit().putBoolean("tutorial_seen", true).apply()
    }

    /** Tutorial language, "en" or "ja". Until one is picked, Japanese on a Japanese phone. */
    fun getTutorialLanguage(): String =
        prefs.getString("tutorial_language", null)
            ?: if (java.util.Locale.getDefault().language == "ja") "ja" else "en"

    fun setTutorialLanguage(code: String) {
        prefs.edit().putString("tutorial_language", code).apply()
    }

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

    /**
     * Our invocation counter, the second half of every GCM IV. One counter for
     * all clients: they share the system title and may share a key, so separate
     * counters would reuse an IV under the same key. Never repeats, also across
     * app restarts.
     */
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