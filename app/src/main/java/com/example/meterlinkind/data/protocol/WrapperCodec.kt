package com.example.meterlinkind.data.protocol

import java.io.ByteArrayOutputStream

/**
 * DLMS/COSEM wrapper (IEC 62056-47) as used by the FT5AB meter:
 *
 *   version (2) | source port (2) | destination port (2) | length (2) | APDU
 *
 * The meter checks that the second version byte is 1..3 and echoes it back,
 * and requires the destination (server) port to be 0x0001
 * (`hrdtushin.c:_wrapperR`). There is no CRC and no link-layer handshake.
 */
object WrapperCodec {
    const val HEADER_SIZE = 8
    const val VERSION = 0x0001
    const val SERVER_PORT = 0x0001

    fun encode(clientPort: Int, apdu: ByteArray, serverPort: Int = SERVER_PORT): ByteArray {
        require(apdu.size <= 0xFFFF) { "APDU too large for wrapper: ${apdu.size}" }
        val out = ByteArray(HEADER_SIZE + apdu.size)
        putU16(out, 0, VERSION)
        putU16(out, 2, clientPort)
        putU16(out, 4, serverPort)
        putU16(out, 6, apdu.size)
        apdu.copyInto(out, HEADER_SIZE)
        return out
    }

    private fun putU16(buf: ByteArray, offset: Int, value: Int) {
        buf[offset] = (value shr 8).toByte()
        buf[offset + 1] = value.toByte()
    }
}

data class WrapperFrame(
    val version: Int,
    val sourcePort: Int,
    val destinationPort: Int,
    val apdu: ByteArray
) {
    override fun equals(other: Any?): Boolean =
        other is WrapperFrame && version == other.version && sourcePort == other.sourcePort &&
            destinationPort == other.destinationPort && apdu.contentEquals(other.apdu)

    override fun hashCode(): Int =
        ((version * 31 + sourcePort) * 31 + destinationPort) * 31 + apdu.contentHashCode()
}

/**
 * Rebuilds wrapper frames from BLE notifications. The DSPS module forwards
 * the meter's 19200-baud UART stream in arbitrary chunks (up to 250 bytes), so
 * one frame may span several notifications and one notification may hold the
 * tail of one frame and the head of the next.
 */
class WrapperAssembler {
    private val buffer = ByteArrayOutputStream()

    /** Feeds a chunk and returns every frame it completes, in order. */
    fun feed(chunk: ByteArray): List<WrapperFrame> {
        buffer.write(chunk)
        val frames = mutableListOf<WrapperFrame>()
        var data = buffer.toByteArray()
        var pos = 0
        while (data.size - pos >= WrapperCodec.HEADER_SIZE) {
            val version = u16(data, pos)
            if (version !in 1..3) {
                // Not a frame start (line noise or a stale tail): resync byte by byte.
                pos++
                continue
            }
            val length = u16(data, pos + 6)
            val end = pos + WrapperCodec.HEADER_SIZE + length
            if (end > data.size) break
            frames += WrapperFrame(
                version = version,
                sourcePort = u16(data, pos + 2),
                destinationPort = u16(data, pos + 4),
                apdu = data.copyOfRange(pos + WrapperCodec.HEADER_SIZE, end)
            )
            pos = end
        }
        if (pos > 0) {
            data = data.copyOfRange(pos, data.size)
            buffer.reset()
            buffer.write(data)
        }
        return frames
    }

    val pendingBytes: Int get() = buffer.size()

    fun reset() = buffer.reset()

    private fun u16(buf: ByteArray, offset: Int): Int =
        ((buf[offset].toInt() and 0xFF) shl 8) or (buf[offset + 1].toInt() and 0xFF)
}
