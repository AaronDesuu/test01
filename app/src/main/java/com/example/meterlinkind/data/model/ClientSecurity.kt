package com.example.meterlinkind.data.model

import com.example.meterlinkind.data.protocol.CipherKeys
import com.example.meterlinkind.data.protocol.ClientProfile

/**
 * Secrets of one DLMS client, each 16 bytes, held as 32 hex digits. They can be
 * entered either as 16 ASCII characters (the IS 15959 convention, so "0" is
 * 0x30) or as the 32 hex digits of the bytes. The authentication password is the
 * LLS password for MR and the HLS secret for US.
 */
data class ClientSecurity(
    val authPassword: String = DEFAULT,
    val blockCipherKey: String = DEFAULT,
    val authenticationKey: String = DEFAULT,
    /** Stored for when the meter's broadcast key is needed; no request uses it yet. */
    val broadcastKey: String = DEFAULT,
) {
    val isValid: Boolean
        get() = listOf(authPassword, blockCipherKey, authenticationKey, broadcastKey).all(::isValidHex)

    val passwordBytes: ByteArray get() = hexToBytes(authPassword)

    val cipherKeys: CipherKeys
        get() = CipherKeys(hexToBytes(blockCipherKey), hexToBytes(authenticationKey))

    companion object {
        /** Bytes in each key. */
        const val KEY_BYTES = 16
        /** Characters when entered as ASCII. */
        const val ASCII_LENGTH = KEY_BYTES
        /** Characters when entered as hex. */
        const val HEX_LENGTH = KEY_BYTES * 2
        /** "0000000000000000" in ASCII, which the meter accepts. */
        const val DEFAULT = "30303030303030303030303030303030"

        fun isValidHex(key: String): Boolean = key.length == HEX_LENGTH && key.all { it.uppercaseChar() in HEX_DIGITS }

        fun isValidAscii(key: String): Boolean = key.length == ASCII_LENGTH && key.all(::isPrintable)

        fun isPrintable(c: Char): Boolean = c.code in 0x20..0x7E

        /** Hex digits of the ASCII characters; partial input converts too. */
        fun asciiToHex(ascii: String): String =
            ascii.toByteArray(Charsets.US_ASCII).joinToString("") { "%02X".format(it) }

        /** The bytes as ASCII, or null if the hex is incomplete or a byte isn't printable. */
        fun hexToAsciiOrNull(hex: String): String? {
            if (hex.length % 2 != 0 || !hex.all { it.uppercaseChar() in HEX_DIGITS }) return null
            val chars = hex.chunked(2).map { it.toInt(16).toChar() }
            return if (chars.all(::isPrintable)) chars.joinToString("") else null
        }

        fun hexToBytes(hex: String): ByteArray = ByteArray(hex.length / 2) { hex.substring(2 * it, 2 * it + 2).toInt(16).toByte() }

        /**
         * A saved value as hex: keys saved before hex entry existed are 16 ASCII
         * characters; newer ones are 32 hex digits. Anything else gives null.
         */
        fun fromStored(value: String): String? = when {
            isValidHex(value.uppercase()) -> value.uppercase()
            isValidAscii(value) -> asciiToHex(value)
            else -> null
        }

        private const val HEX_DIGITS = "0123456789ABCDEF"
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
