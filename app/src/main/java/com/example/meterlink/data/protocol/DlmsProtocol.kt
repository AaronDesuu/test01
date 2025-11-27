package com.example.meterlink.data.protocol

import android.util.Log
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

class DlmsProtocol {
    private val TAG = "DlmsProtocol"
    private val hdlc = HdlcLayer()

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
    private var currentRank: Int = 4  // PUBLIC by default

    // Keys
    private var globalKey: ByteArray? = null
    private var dedicatedKey: ByteArray? = null

    // Static protocol bytes
    companion object {
        private val GET_REQUEST = byteArrayOf(0xc0.toByte(), 0x01, 0xc1.toByte())
        private val SET_REQUEST = byteArrayOf(0xc1.toByte(), 0x01, 0xc1.toByte())
        private val ACTION_REQUEST = byteArrayOf(0xc3.toByte(), 0x01, 0xc1.toByte())
        private val GET_NEXT_REQUEST = byteArrayOf(0xc0.toByte(), 0x02, 0x00, 0x00, 0x00, 0x00)
        private val AARQ_TAG = byteArrayOf(0x60.toByte())
        private val RLRQ_TAG = byteArrayOf(0x62.toByte())

        // User ranks
        const val RANK_SUPER = 0
        const val RANK_ADMIN = 1
        const val RANK_POWER = 2
        const val RANK_READER = 3
        const val RANK_PUBLIC = 4
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

    /**
     * Open HDLC connection (SNRM)
     */
    fun createOpenRequest(): ByteArray {
        return hdlc.hdlcs(0x93.toByte(), null)
    }

    /**
     * Process open response (UA)
     */
    fun processOpenResponse(response: ByteArray): DlmsResult {
        return when (val result = hdlc.hdlcr(response)) {
            is HdlcResult.Success -> DlmsResult.Success("Connection opened")
            is HdlcResult.Error -> DlmsResult.Error(result.message)
        }
    }

    /**
     * Create AARQ (Association Request)
     */
    fun createSessionRequest(password: ByteArray? = null): ByteArray {
        // Simplified AARQ - would need full implementation based on rank
        val aarq = byteArrayOf(
            0x60.toByte(), 0x30, // AARQ tag + length
            0xa1.toByte(), 0x09, // Application context
            0x06, 0x07, 0x60, 0x85.toByte(), 0x74, 0x05, 0x08, 0x01, 0x01,
            0x8a.toByte(), 0x02, 0x07, 0x80.toByte() // Authentication: none
        )

        return hdlc.hdlcs(0x13, aarq)
    }

    /**
     * Process session response (AARE)
     */
    fun processSessionResponse(response: ByteArray): DlmsResult {
        return when (val result = hdlc.hdlcr(response)) {
            is HdlcResult.Success -> {
                result.data?.let { data ->
                    if (data[0] == 0x61.toByte()) {
                        DlmsResult.Success("Session established")
                    } else {
                        DlmsResult.Error("Invalid AARE response")
                    }
                } ?: DlmsResult.Error("No data in response")
            }
            is HdlcResult.Error -> DlmsResult.Error(result.message)
        }
    }

    /**
     * Create GET request with HDLC wrapping
     */
    fun createWrappedGetRequest(
        objIndex: Int,
        attribute: Byte,
        selector: Byte = 0,
        parameters: ByteArray? = null
    ): ByteArray {
        val request = createGetRequest(objIndex, attribute, selector, parameters)
        return hdlc.hdlcs(0x13, request)
    }

    /**
     * Process GET response
     */
    fun processGetResponse(response: ByteArray): DlmsResult {
        return when (val result = hdlc.hdlcr(response)) {
            is HdlcResult.Success -> {
                result.data?.let { data ->
                    // Decrypt if needed
                    val decryptedData = if (currentRank <= RANK_ADMIN && dedicatedKey != null) {
                        decrypt(dedicatedKey!!, data) ?: return DlmsResult.Error("Decryption failed")
                    } else {
                        data
                    }

                    // Parse response
                    if (decryptedData.size > 3 && decryptedData[0] == 0xc4.toByte()) {
                        DlmsResult.DataReceived(decryptedData)
                    } else {
                        DlmsResult.Error("Invalid GET response format")
                    }
                } ?: DlmsResult.Error("No data in response")
            }
            is HdlcResult.Error -> DlmsResult.Error(result.message)
        }
    }

    /**
     * Close connection
     */
    fun createCloseRequest(): ByteArray {
        val rlrq = byteArrayOf(0x62.toByte(), 0x03, 0x80.toByte(), 0x01, 0x00)
        return hdlc.hdlcs(0x13, rlrq)
    }

    fun setRank(rank: Int) {
        currentRank = rank
    }
}

sealed class DlmsResult {
    data class Success(val message: String) : DlmsResult()
    data class DataReceived(val data: ByteArray) : DlmsResult()
    data class Error(val message: String) : DlmsResult()
}