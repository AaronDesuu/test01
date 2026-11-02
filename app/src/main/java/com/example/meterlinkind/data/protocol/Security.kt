package com.example.meterlinkind.data.protocol

import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

class SecurityException(message: String) : Exception(message)

/** AES-128 keys of one client: global unicast (block cipher) key and authentication key. */
class CipherKeys(val blockCipherKey: ByteArray, val authenticationKey: ByteArray) {
    init {
        require(blockCipherKey.size == 16 && authenticationKey.size == 16) { "AES-128 keys are 16 bytes" }
    }
}

/**
 * DLMS security suite 0 (AES-GCM-128) as the FT5AB applies it (`aes_gcm.c`):
 *
 *   SC (1) | frame counter (4) | ciphertext | tag (12, only if SC has 0x10)
 *
 * IV = sender system title (8) | frame counter, AAD = SC | authentication key.
 */
object SecuritySuite0 {
    const val SC_AUTHENTICATED = 0x10
    const val SC_ENCRYPTED = 0x20
    const val TAG_LENGTH = 12
    const val SYSTEM_TITLE_LENGTH = 8

    /** Returns the protected payload `SC | FC | ciphertext [| tag]`. */
    fun encrypt(
        securityControl: Int,
        systemTitle: ByteArray,
        frameCounter: Long,
        keys: CipherKeys,
        plaintext: ByteArray,
    ): ByteArray {
        val fc = u32(frameCounter)
        val iv = iv(systemTitle, fc)
        val body = if (securityControl and SC_AUTHENTICATED != 0) {
            gcm(Cipher.ENCRYPT_MODE, keys, iv, securityControl).doFinal(plaintext)
        } else {
            ctr(keys, iv).doFinal(plaintext)
        }
        return byteArrayOf(securityControl.toByte()) + fc + body
    }

    /** Reverses [encrypt]; [systemTitle] is the sender's. Throws if the tag doesn't match. */
    fun decrypt(systemTitle: ByteArray, keys: CipherKeys, protected: ByteArray): ByteArray {
        if (protected.size < 1 + 4) throw SecurityException("Ciphered APDU too short")
        val sc = protected[0].toInt() and 0xFF
        if (sc and (SC_AUTHENTICATED or SC_ENCRYPTED) == 0) {
            throw SecurityException("Unsupported security control 0x%02x".format(sc))
        }
        val iv = iv(systemTitle, protected.copyOfRange(1, 5))
        val body = protected.copyOfRange(5, protected.size)
        return if (sc and SC_AUTHENTICATED != 0) {
            if (body.size < TAG_LENGTH) throw SecurityException("Ciphered APDU has no room for a tag")
            try {
                gcm(Cipher.DECRYPT_MODE, keys, iv, sc).doFinal(body)
            } catch (e: javax.crypto.AEADBadTagException) {
                throw SecurityException("Authentication tag mismatch: wrong block cipher or authentication key")
            }
        } else {
            ctr(keys, iv).doFinal(body)
        }
    }

    /** One AES-128 block, as HLS mechanism 2 uses it: f(challenge) = AES(secret, challenge). */
    fun aesEcb(key: ByteArray, block: ByteArray): ByteArray {
        require(key.size == 16 && block.size == 16) { "AES-ECB takes a 16-byte key and block" }
        return Cipher.getInstance("AES/ECB/NoPadding").run {
            init(Cipher.ENCRYPT_MODE, SecretKeySpec(key, "AES"))
            doFinal(block)
        }
    }

    private fun gcm(mode: Int, keys: CipherKeys, iv: ByteArray, sc: Int): Cipher =
        Cipher.getInstance("AES/GCM/NoPadding").apply {
            init(mode, SecretKeySpec(keys.blockCipherKey, "AES"), GCMParameterSpec(TAG_LENGTH * 8, iv))
            updateAAD(byteArrayOf(sc.toByte()) + keys.authenticationKey)
        }

    /** Encryption-only: GCM without the tag is CTR mode starting at counter block IV | 00000002. */
    private fun ctr(keys: CipherKeys, iv: ByteArray): Cipher =
        Cipher.getInstance("AES/CTR/NoPadding").apply {
            init(Cipher.ENCRYPT_MODE, SecretKeySpec(keys.blockCipherKey, "AES"), IvParameterSpec(iv + u32(2)))
        }

    private fun iv(systemTitle: ByteArray, frameCounter: ByteArray): ByteArray {
        if (systemTitle.size != SYSTEM_TITLE_LENGTH) throw SecurityException("System title must be 8 bytes")
        return systemTitle + frameCounter
    }

    private fun u32(v: Long) = byteArrayOf((v shr 24).toByte(), (v shr 16).toByte(), (v shr 8).toByte(), v.toByte())
}

/**
 * Wraps and unwraps xDLMS APDUs in global ciphering (`glo-…` tags) for one
 * association. The meter answers with the security control we sent and its
 * own system title, which arrives in the AARE ([serverSystemTitle]).
 */
class SecureChannel(
    val clientSystemTitle: ByteArray,
    private val keys: CipherKeys,
    private val securityControl: Int,
    private val nextFrameCounter: () -> Long,
) {
    var serverSystemTitle: ByteArray? = null

    fun protect(apdu: ByteArray): ByteArray {
        val tag = GLOBAL_TAGS[apdu.firstOrNull()?.toInt()?.and(0xFF)]
            ?: throw SecurityException("No ciphered form for ${describeApdu(apdu)}")
        val payload = SecuritySuite0.encrypt(securityControl, clientSystemTitle, nextFrameCounter(), keys, apdu)
        return byteArrayOf(tag.toByte()) + Axdr.encodeLength(payload.size) + payload
    }

    /**
     * Deciphers a `glo-…` or general-glo-ciphering reply; anything else
     * (exception response, service error) passes through.
     */
    fun unprotect(apdu: ByteArray): ByteArray {
        val tag = apdu.firstOrNull()?.toInt()?.and(0xFF)
        val r = Axdr.Reader(apdu, 1)
        return when (tag) {
            in GLOBAL_TAGS.values -> {
                val title = serverSystemTitle ?: throw SecurityException("Meter system title unknown")
                SecuritySuite0.decrypt(title, keys, r.bytes(r.length()))
            }
            // general-glo-ciphering carries the sender's system title itself.
            TAG_GENERAL_GLO_CIPHERING -> {
                val title = r.bytes(r.length())
                SecuritySuite0.decrypt(title, keys, r.bytes(r.length()))
            }
            else -> apdu
        }
    }

    companion object {
        const val TAG_GENERAL_GLO_CIPHERING = 0xDB

        /** Plain APDU tag → its global-ciphering counterpart (IEC 62056-5-3 §8). */
        val GLOBAL_TAGS = mapOf(
            0x01 to 0x21, // initiate-request
            0x08 to 0x28, // initiate-response
            0xC0 to 0xC8, // get-request
            0xC1 to 0xC9, // set-request
            0xC3 to 0xCB, // action-request
            0xC4 to 0xCC, // get-response
            0xC5 to 0xCD, // set-response
            0xC7 to 0xCF, // action-response
        )
    }
}
