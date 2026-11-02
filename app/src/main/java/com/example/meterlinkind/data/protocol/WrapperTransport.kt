package com.example.meterlinkind.data.protocol

/** Raw byte pipe to the meter (the BLE DSPS link in the app, a fake in tests). */
interface ByteLink {
    suspend fun send(data: ByteArray): Boolean
    /** Next received chunk, or null after [timeoutMs] with nothing. */
    suspend fun receive(timeoutMs: Long): ByteArray?
    /** Drops anything already received. */
    fun clear()
}

class TransportException(message: String) : Exception(message)

/** IEC 62056-47 wrapper: no link layer, the AARQ is the first frame. */
class WrapperTransport(
    private val link: ByteLink,
    private val clientPort: Int,
) {
    private val assembler = WrapperAssembler()

    /** Sends one APDU and returns the APDU of the meter's reply. */
    suspend fun request(apdu: ByteArray): ByteArray {
        link.clear()
        assembler.reset()
        if (!link.send(WrapperCodec.encode(clientPort, apdu))) throw TransportException("Failed to send request")
        var timeout = RESPONSE_TIMEOUT_MS
        while (true) {
            val chunk = link.receive(timeout) ?: throw TransportException(
                if (assembler.pendingBytes == 0) "No response from meter"
                else "Incomplete response (${assembler.pendingBytes} bytes)"
            )
            assembler.feed(chunk).firstOrNull { it.destinationPort == clientPort }?.let { return it.apdu }
            timeout = INTER_CHUNK_TIMEOUT_MS
        }
    }

    companion object {
        /** At 19200 baud the meter needs ~0.5 ms per byte; allow for its processing on top. */
        const val RESPONSE_TIMEOUT_MS = 5000L
        /** Gap allowed between chunks of one reply once it has started arriving. */
        const val INTER_CHUNK_TIMEOUT_MS = 1500L
    }
}
