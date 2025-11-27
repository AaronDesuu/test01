package com.example.meterlink.data.protocol

import android.util.Log
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

class DlmsProtocol {
    private val TAG = "DlmsProtocol"

    // Security state
    private var svChallenge: ByteArray? = null
    private var clChallenge: ByteArray? = null
    private var svAppTitle: ByteArray? = null
    private var clAppTitle: ByteArray? = null
    private var frameCounter: Int = 0

    // Request state
    private var blockNo: Int = 0
    private var currentObj: Int = 0
    private var currentMode: Byte = 0
    private var currentAtr: Byte = 0
    private var currentSel: Byte = 0

    // Static protocol bytes
    companion object {
        private val GET_REQUEST = byteArrayOf(0xc0.toByte(), 0x01, 0xc1.toByte())
        private val SET_REQUEST = byteArrayOf(0xc1.toByte(), 0x01, 0xc1.toByte())
        private val ACTION_REQUEST = byteArrayOf(0xc3.toByte(), 0x01, 0xc1.toByte())
        private val GET_NEXT_REQUEST = byteArrayOf(0xc0.toByte(), 0x02, 0x00, 0x00, 0x00, 0x00)
    }

    /**
     * Encrypt data using AES-GCM
     */
    fun encrypt(keyData: ByteArray, securityControl: Byte, input: ByteArray): ByteArray {
        val output = ByteArray(5 + input.size + 12) // SC + FC + data + tag

        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        val key = SecretKeySpec(keyData, "AES")

        output[0] = securityControl
        updateFrameCounter(output, 1)

        // Build IV: clientAppTitle + frameCounter
        val iv = ByteArray(clAppTitle!!.size + 4)
        System.arraycopy(clAppTitle!!, 0, iv, 0, clAppTitle!!.size)
        System.arraycopy(output, 1, iv, clAppTitle!!.size, 4)

        cipher.init(Cipher.ENCRYPT_MODE, key, GCMParameterSpec(96, iv))

        // Update AAD with challenge
        val challenge = if (output[0] == 0x10.toByte()) svChallenge!! else clChallenge!!
        challenge[0] = output[0]
        cipher.updateAAD(challenge)

        val encrypted = cipher.doFinal(input)
        System.arraycopy(encrypted, 0, output, 5, encrypted.size)

        return output
    }

    /**
     * Decrypt data using AES-GCM
     */
    fun decrypt(keyData: ByteArray, input: ByteArray): ByteArray? {
        try {
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            val key = SecretKeySpec(keyData, "AES")

            // Build IV: serverAppTitle + frameCounter from input
            val iv = ByteArray(svAppTitle!!.size + 4)
            System.arraycopy(svAppTitle!!, 0, iv, 0, svAppTitle!!.size)
            System.arraycopy(input, 1, iv, svAppTitle!!.size, 4)

            cipher.init(Cipher.DECRYPT_MODE, key, GCMParameterSpec(96, iv))

            // Update AAD with challenge
            val challenge = if (input[0] == 0x10.toByte()) clChallenge!! else svChallenge!!
            challenge[0] = input[0]
            cipher.updateAAD(challenge)

            return cipher.doFinal(input, 5, input.size - 5)
        } catch (e: Exception) {
            Log.e(TAG, "Decryption failed", e)
            return null
        }
    }

    /**
     * Create GET request
     */
    fun createGetRequest(
        objIndex: Int,
        attribute: Byte,
        selector: Byte = 0,
        parameters: ByteArray? = null,
        position: Byte = 0
    ): ByteArray {
        if (blockNo == 0) {
            currentObj = objIndex
            currentMode = 0
            currentAtr = attribute
            currentSel = selector

            var length = GET_REQUEST.size + 7 // base + object identifier
            if (selector > 0) length++
            if (parameters != null) length += parameters.size

            val request = ByteArray(length)
            System.arraycopy(GET_REQUEST, 0, request, 0, GET_REQUEST.size)

            // Add object identifier (placeholder - would need object registry)
            var offset = GET_REQUEST.size + 7
            request[offset++] = attribute

            if (selector > 0) {
                request[offset++] = 0x01
                request[offset++] = selector
            }

            parameters?.let {
                System.arraycopy(it, 0, request, offset, it.size)
            }

            return request
        } else {
            val request = ByteArray(GET_NEXT_REQUEST.size)
            System.arraycopy(GET_NEXT_REQUEST, 0, request, 0, GET_NEXT_REQUEST.size)
            setUInt32(request, request.size - 4, blockNo)
            return request
        }
    }

    /**
     * Create SET request
     */
    fun createSetRequest(
        objIndex: Int,
        attribute: Byte,
        selector: Byte = 0,
        parameters: ByteArray?,
        position: Byte = 0
    ): ByteArray {
        var length = SET_REQUEST.size + 7
        if (selector > 0) length++
        if (parameters != null) length += parameters.size

        val request = ByteArray(length)
        System.arraycopy(SET_REQUEST, 0, request, 0, SET_REQUEST.size)

        var offset = SET_REQUEST.size + 7
        request[offset++] = attribute

        if (selector > 0) {
            request[offset++] = 0x01
            request[offset++] = selector
        }

        parameters?.let {
            System.arraycopy(it, 0, request, offset, it.size)
        }

        currentObj = objIndex
        currentMode = 1
        currentAtr = attribute
        currentSel = selector

        return request
    }

    /**
     * Create ACTION request
     */
    fun createActionRequest(
        objIndex: Int,
        method: Byte,
        parameters: ByteArray?,
        position: Byte = 0
    ): ByteArray {
        var length = ACTION_REQUEST.size + 7
        if (parameters != null) {
            length++
            length += parameters.size
        }

        val request = ByteArray(length)
        System.arraycopy(ACTION_REQUEST, 0, request, 0, ACTION_REQUEST.size)

        var offset = ACTION_REQUEST.size + 7
        request[offset++] = method

        if (parameters != null) {
            request[offset++] = 0x01
            System.arraycopy(parameters, 0, request, offset, parameters.size)
        }

        currentObj = objIndex
        currentMode = 3
        currentAtr = method
        currentSel = 0

        return request
    }

    // Helper methods
    fun setClientAppTitle(data: ByteArray, offset: Int) {
        clAppTitle = ByteArray(8)
        System.arraycopy(data, offset, clAppTitle!!, 0, 8)
    }

    fun setServerAppTitle(data: ByteArray, offset: Int) {
        svAppTitle = ByteArray(8)
        System.arraycopy(data, offset, svAppTitle!!, 0, 8)
    }

    fun setClientChallenge(data: ByteArray, offset: Int) {
        clChallenge = ByteArray(32)
        System.arraycopy(data, offset, clChallenge!!, 1, 31)
    }

    fun setServerChallenge(data: ByteArray, offset: Int) {
        svChallenge = ByteArray(32)
        System.arraycopy(data, offset, svChallenge!!, 1, 31)
    }

    private fun updateFrameCounter(output: ByteArray, offset: Int) {
        frameCounter++
        setUInt32(output, offset, frameCounter)
    }

    private fun setUInt32(array: ByteArray, offset: Int, value: Int) {
        array[offset] = (value shr 24).toByte()
        array[offset + 1] = (value shr 16).toByte()
        array[offset + 2] = (value shr 8).toByte()
        array[offset + 3] = value.toByte()
    }

    fun hexStringToByteArray(hex: String): ByteArray {
        val len = hex.length
        val data = ByteArray(len / 2)
        var i = 0
        while (i < len) {
            data[i / 2] = ((Character.digit(hex[i], 16) shl 4)
                    + Character.digit(hex[i + 1], 16)).toByte()
            i += 2
        }
        return data
    }

    fun byteArrayToHexString(bytes: ByteArray): String {
        return bytes.joinToString("") { "%02x".format(it) }
    }
}