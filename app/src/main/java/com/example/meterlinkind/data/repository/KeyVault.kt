package com.example.meterlinkind.data.repository

import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Base64
import java.security.KeyStore
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec

/**
 * Encrypts the meter keys before they are saved. The AES key lives in the
 * Android Keystore and can't be read out of the phone, so a copied settings
 * file is useless anywhere else.
 *
 * A stored value is `v1:` + Base64(IV | ciphertext | tag), AES-256-GCM. The
 * preference name is the AAD, so a value moved to another slot fails to open.
 */
internal class KeyVault {
    fun seal(name: String, plain: String): String {
        val cipher = Cipher.getInstance(TRANSFORMATION)
        cipher.init(Cipher.ENCRYPT_MODE, key())
        cipher.updateAAD(name.toByteArray())
        val sealed = cipher.iv + cipher.doFinal(plain.toByteArray())
        return PREFIX + Base64.encodeToString(sealed, Base64.NO_WRAP)
    }

    /** The saved text, or null if [stored] isn't ours or can't be opened (Keystore key gone, file from another phone). */
    fun open(name: String, stored: String): String? {
        if (!stored.startsWith(PREFIX)) return null
        return runCatching {
            val bytes = Base64.decode(stored.removePrefix(PREFIX), Base64.NO_WRAP)
            val cipher = Cipher.getInstance(TRANSFORMATION)
            cipher.init(Cipher.DECRYPT_MODE, key(), GCMParameterSpec(TAG_BITS, bytes, 0, IV_SIZE))
            cipher.updateAAD(name.toByteArray())
            String(cipher.doFinal(bytes, IV_SIZE, bytes.size - IV_SIZE))
        }.getOrNull()
    }

    private fun key(): SecretKey {
        val store = KeyStore.getInstance(KEYSTORE).apply { load(null) }
        (store.getKey(ALIAS, null) as? SecretKey)?.let { return it }
        val generator = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, KEYSTORE)
        generator.init(
            KeyGenParameterSpec.Builder(ALIAS, KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT)
                .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
                .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                .setKeySize(256)
                .build()
        )
        return generator.generateKey()
    }

    companion object {
        private const val KEYSTORE = "AndroidKeyStore"
        private const val ALIAS = "meter_keys"
        private const val TRANSFORMATION = "AES/GCM/NoPadding"
        private const val PREFIX = "v1:"
        private const val IV_SIZE = 12
        private const val TAG_BITS = 128
    }
}
