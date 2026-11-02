package com.example.meterlinkind.data.model

import com.example.meterlinkind.data.protocol.CipherKeys
import com.example.meterlinkind.data.protocol.ClientProfile

/**
 * Secrets of one DLMS client, each 16 ASCII characters (16 bytes, so "0" is
 * 0x30), the IS 15959 convention. The authentication password is the LLS
 * password for MR and the HLS secret for US.
 */
data class ClientSecurity(
    val authPassword: String = DEFAULT,
    val blockCipherKey: String = DEFAULT,
    val authenticationKey: String = DEFAULT,
    /** Stored for when the meter's broadcast key is needed; no request uses it yet. */
    val broadcastKey: String = DEFAULT,
) {
    val isValid: Boolean
        get() = listOf(authPassword, blockCipherKey, authenticationKey, broadcastKey).all(::isValidKey)

    val passwordBytes: ByteArray get() = authPassword.toByteArray(Charsets.US_ASCII)

    val cipherKeys: CipherKeys
        get() = CipherKeys(
            blockCipherKey.toByteArray(Charsets.US_ASCII),
            authenticationKey.toByteArray(Charsets.US_ASCII),
        )

    companion object {
        const val KEY_LENGTH = 16
        const val DEFAULT = "0000000000000000"

        fun isValidKey(key: String): Boolean = key.length == KEY_LENGTH && key.all { it.code in 0x20..0x7E }
    }
}

/**
 * App logins. Each opens the app as one DLMS client. Placeholders until the
 * real accounts are decided.
 */
object AppAccounts {
    private val ACCOUNTS = mapOf(
        ("admin" to "admin") to ClientProfile.UTILITY_SETTINGS,
        ("reader" to "reader") to ClientProfile.METER_READER,
    )

    /** The client this login opens, or null if the username/password is wrong. */
    fun clientFor(username: String, password: String): ClientProfile? = ACCOUNTS[username.trim() to password]
}
