package com.example.meterlink.data.protocol

import android.util.Log

class HdlcLayer(
    private val serverAddress: Int = 0x03,  // Server
    private val clientAddress: Int = 0x10   // Public Client
) {
    private val TAG = "HdlcLayer"
    private var ns: Int = 0  // Send sequence number
    private var nr: Int = 0  // Receive sequence number

    companion object {
        private const val HDLC_FLAG: Byte = 0x7E
    }

    /**
     * Create HDLC frame (send)
     */
    fun hdlcs(cmd: Byte, llc: ByteArray?): ByteArray {
        val baseLength = if (llc != null) 12 + llc.size else 9
        val frame = ByteArray(baseLength + 2)
        var offset = 0

        frame[offset++] = HDLC_FLAG

        val length = baseLength - 2
        frame[offset++] = (0xa0 or (length shr 8)).toByte()
        frame[offset++] = (length and 0xff).toByte()

        // Destination and source addresses
        frame[offset++] = serverAddress.toByte()
        frame[offset++] = clientAddress.toByte()
        frame[offset++] = cmd

        var crc = calculateCrc16(frame, offset - 1)
        frame[offset++] = (crc shr 8).toByte()
        frame[offset++] = (crc and 0xff).toByte()

        if (llc != null) {
            frame[offset++] = 0xE6.toByte()
            frame[offset++] = 0xE6.toByte()
            frame[offset++] = 0x00
            System.arraycopy(llc, 0, frame, offset, llc.size)
            offset += llc.size

            crc = calculateCrc16(frame, offset - 1)
            frame[offset++] = (crc shr 8).toByte()
            frame[offset++] = (crc and 0xff).toByte()
        }

        frame[offset++] = HDLC_FLAG

        // Return only filled bytes
        return frame.copyOf(offset)
    }

    /**
     * Parse HDLC frame (receive)
     */
    fun hdlcr(input: ByteArray): HdlcResult {
        if (input.size < 9) {
            return HdlcResult.Error("Frame too short")
        }

        var offset = 1  // Skip start flag

        // Parse length
        val length = ((input[offset++].toInt() and 0x0F) shl 8) or
                (input[offset++].toInt() and 0xFF)

        if (input.size != length + offset - 1) {
            return HdlcResult.Error("Invalid frame length")
        }

        // Verify destination address (should be client address in response)
        if (input[offset++] != clientAddress.toByte()) {
            return HdlcResult.Error("Address mismatch")
        }

        // Skip source address
        offset++

        // Control field
        val control = input[offset++]

        // Verify HCS
        val hcsCrc = calculateCrc16(input, offset - 1)
        val hcsHigh = input[offset++]
        val hcsLow = input[offset++]

        if (hcsHigh != (hcsCrc shr 8).toByte() ||
            hcsLow != (hcsCrc and 0xFF).toByte()) {
            return HdlcResult.Error("HCS verification failed")
        }

        // Extract LLC data if present
        if (length > 12) {
            val llcLength = length - 12
            offset += 3  // Skip LLC header

            val llcData = ByteArray(llcLength)
            System.arraycopy(input, offset, llcData, 0, llcLength)
            offset += llcLength

            // Verify FCS
            val fcsCrc = calculateCrc16(input, offset - 1)
            val fcsHigh = input[offset++]
            val fcsLow = input[offset]

            if (fcsHigh != (fcsCrc shr 8).toByte() ||
                fcsLow != (fcsCrc and 0xFF).toByte()) {
                return HdlcResult.Error("FCS verification failed")
            }

            return HdlcResult.Success(control, llcData)
        }

        return HdlcResult.Success(control, null)
    }

    /**
     * Calculate CRC16 for HDLC
     */
    private fun calculateCrc16(data: ByteArray, length: Int): Int {
        val polynomial = 0x1021
        var crc = 0xFFFF

        for (i in 1..length) {
            crc = crc xor ((data[i].toInt() and 0xFF) shl 8)
            for (j in 0 until 8) {
                if ((crc and 0x8000) != 0) {
                    crc = (crc shl 1) xor polynomial
                } else {
                    crc = crc shl 1
                }
            }
        }

        return crc and 0xFFFF xor 0xFFFF
    }
}

sealed class HdlcResult {
    data class Success(val control: Byte, val data: ByteArray?) : HdlcResult()
    data class Error(val message: String) : HdlcResult()
}