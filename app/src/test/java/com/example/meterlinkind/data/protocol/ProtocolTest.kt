package com.example.meterlinkind.data.protocol

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class ProtocolTest {

    private fun hex(s: String): ByteArray =
        s.replace(" ", "").chunked(2).map { it.toInt(16).toByte() }.toByteArray()

    // --- Wrapper -----------------------------------------------------------

    @Test
    fun wrapperHeaderUsesPublicPortAndServerPortOne() {
        val frame = WrapperCodec.encode(ClientProfile.PUBLIC.clientPort, hex("c0 01 c1"))
        assertArrayEquals(hex("0001 0010 0001 0003 c001c1"), frame)
    }

    @Test
    fun assemblerJoinsFrameSplitAcrossNotifications() {
        val a = WrapperAssembler()
        val frame = hex("0001 0001 0010 0005 c401c10011")
        assertTrue(a.feed(frame.copyOfRange(0, 3)).isEmpty())
        assertTrue(a.feed(frame.copyOfRange(3, 10)).isEmpty())
        val out = a.feed(frame.copyOfRange(10, frame.size))
        assertEquals(1, out.size)
        assertEquals(0x0001, out[0].sourcePort)
        assertEquals(0x0010, out[0].destinationPort)
        assertArrayEquals(hex("c401c10011"), out[0].apdu)
        assertEquals(0, a.pendingBytes)
    }

    @Test
    fun assemblerSplitsTwoFramesInOneChunkAndKeepsTail() {
        val a = WrapperAssembler()
        val out = a.feed(hex("0001 0001 0010 0001 aa  0001 0001 0010 0002 bbcc  0001 00"))
        assertEquals(2, out.size)
        assertArrayEquals(hex("aa"), out[0].apdu)
        assertArrayEquals(hex("bbcc"), out[1].apdu)
        assertEquals(3, a.pendingBytes)
    }

    @Test
    fun assemblerSkipsGarbageBeforeFrame() {
        val a = WrapperAssembler()
        val out = a.feed(hex("7e a0 ff  0001 0001 0010 0001 aa"))
        assertEquals(1, out.size)
        assertArrayEquals(hex("aa"), out[0].apdu)
    }

    // --- Association -------------------------------------------------------

    // Gurux DLMS Director, PC client, wrapper over the optical probe (2026-10-31 11:54:59).
    private val guruxAarqFrame = hex(
        "00 01 00 10 00 01 00 1F 60 1D A1 09 06 07 60 85 74 05 08 01 01 BE 10 04 0E 01 00 00 00 06" +
            " 5F 1F 04 00 62 1E 5D FF FF"
    )
    private val guruxAareFrame = hex(
        "00 01 00 01 00 10 00 2B 61 29 A1 09 06 07 60 85 74 05 08 01 01 A2 03 02 01 00 A3 05 A1 03" +
            " 02 01 00 BE 10 04 0E 08 00 06 5F 1F 04 00 00 00 10 02 E2 00 07"
    )

    @Test
    fun publicAarqFrameIsByteIdenticalToGurux() {
        // Only A1 (LN, no ciphering) and BE (user information): the meter rejects
        // any other tag for the Public client.
        assertArrayEquals(
            guruxAarqFrame,
            WrapperCodec.encode(ClientProfile.PUBLIC.clientPort, Association.publicRequest())
        )
    }

    @Test
    fun parsesAcceptedAare() {
        val aare = hex(
            "61 29" +
                "a1 09 06 07 60 85 74 05 08 01 01" +
                "a2 03 02 01 00" +
                "a3 05 a1 03 02 01 00" +
                "be 10 04 0e 08 00 06 5f 1f 04 00 00 00 10 04 00 00 07"
        )
        val r = Association.parseResponse(aare)
        assertTrue(r.accepted)
        assertEquals(Conformance.GET, r.negotiatedConformance)
        assertEquals(0x0400, r.serverMaxReceivePdu)
        assertEquals(6, r.dlmsVersion)
    }

    @Test
    fun parsesRejectedAare() {
        val aare = hex(
            "61 1f" +
                "a1 09 06 07 60 85 74 05 08 01 01" +
                "a2 03 02 01 01" +
                "a3 05 a1 03 02 01 0d" +
                "be 06 04 04 0e 01 06 02"
        )
        val r = Association.parseResponse(aare)
        assertFalse(r.accepted)
        assertEquals(1, r.result)
        assertEquals(13, r.diagnostic)
        assertTrue(r.reason, r.reason.startsWith("InitiateRequest refused"))
    }

    @Test
    fun releaseRequestCarriesReasonNormal() {
        // acse.c:request_release_reason wants exactly 80 01 00 after the length.
        assertArrayEquals(hex("62 03 80 01 00"), Association.releaseRequest())
    }

    @Test
    fun parsesRlre() {
        assertTrue(Association.parseReleaseResponse(hex("63 03 80 01 00")))
        assertFalse(Association.parseReleaseResponse(hex("63 03 80 01 01")))
        assertEquals(
            "Expected RLRE (0x63), got 0x61 (6100)",
            runCatching { Association.parseReleaseResponse(hex("61 00")) }.exceptionOrNull()?.message
        )
    }

    @Test
    fun conformanceBitsMatchMeterTable() {
        // xdlms.c CONFORMANCE[] Public row: 00 00 10 -> GET only.
        assertEquals(0x000010, Conformance.GET)
        assertEquals(0x001000, Conformance.BLOCK_TRANSFER_WITH_GET)
        assertEquals(0x200000, Conformance.GENERAL_BLOCK_TRANSFER)
        // The meter rejects SN read/write/unconfirmed-write (first byte & 0x1c).
        assertEquals(0, (Conformance.PROPOSED shr 16) and 0x1c)
    }

    // --- GET ---------------------------------------------------------------

    @Test
    fun getRequestForSerialNumber() {
        assertArrayEquals(
            hex("c0 01 c1 0001 00 00 60 01 00 ff 02 00"),
            Get.request(PublicObjects.SERIAL_NUMBER)
        )
    }

    @Test
    fun getRequestForPcAssociation() {
        assertArrayEquals(
            hex("c0 01 c1 000f 00 00 28 00 01 ff 02 00"),
            Get.request(PublicObjects.PC_ASSOCIATION)
        )
    }

    @Test
    fun parsesGetResponseWithOctetString() {
        val r = Get.parseResponse(hex("c4 01 c1 00 09 08") + "12345678".toByteArray())
        assertEquals("12345678", (r as GetResult.Data).data.display())
    }

    @Test
    fun parsesGetResponseWithAccessError() {
        val r = Get.parseResponse(hex("c4 01 c1 01 04"))
        assertEquals(4, (r as GetResult.AccessError).code)
    }

    @Test
    fun parsesGetResponseBlock() {
        val r = Get.parseResponse(hex("c4 02 c1 00 00000001 00 03 090148"))
        r as GetResult.Block
        assertFalse(r.last)
        assertEquals(1L, r.blockNumber)
        assertArrayEquals(hex("090148"), r.raw)
        assertArrayEquals(hex("c0 02 c1 00000001"), Get.nextBlock(1))
    }

    @Test
    fun parsesExceptionResponse() {
        val r = Get.parseResponse(hex("d8 01 02"))
        assertTrue(r is GetResult.ServiceFailure)
    }

    // --- A-XDR -------------------------------------------------------------

    @Test
    fun decodesSignedAndUnsignedIntegers() {
        assertEquals(-2L, (Axdr.decode(hex("10 fffe")) as DlmsData.Integer).value)
        assertEquals(65534L, (Axdr.decode(hex("12 fffe")) as DlmsData.Integer).value)
        assertEquals(4294967295L, (Axdr.decode(hex("06 ffffffff")) as DlmsData.Integer).value)
    }

    @Test
    fun decodesClockOctetString() {
        val clock = Axdr.decode(hex("09 0c 07ea 0a 1f 06 0e 1e 00 ff 8000 00"))
        assertEquals("2026-10-31 14:30:00", clock.display())
    }

    @Test
    fun decodesNestedStructureWithLongLength() {
        val data = Axdr.decode(hex("01 02 02 02 11 01 0a 01 41 02 02 11 02 0a 01 42"))
        assertEquals("[{1, A}, {2, B}]", data.display())
        val long = Axdr.decode(hex("09 81 80") + ByteArray(0x80) { 0x41 })
        assertEquals(0x80, (long as DlmsData.OctetString).bytes.size)
    }

    // --- Transport ---------------------------------------------------------

    /** Answers each send with the next scripted reply, split across BLE-sized chunks. */
    private class ScriptedLink(vararg replies: ByteArray) : ByteLink {
        private val pending = ArrayDeque(replies.toList())
        private val inbox = ArrayDeque<ByteArray>()
        val sent = mutableListOf<ByteArray>()

        override suspend fun send(data: ByteArray): Boolean {
            sent += data
            pending.removeFirstOrNull()?.let { r -> r.toList().chunked(5).forEach { inbox += it.toByteArray() } }
            return true
        }

        override suspend fun receive(timeoutMs: Long): ByteArray? = inbox.removeFirstOrNull()
        override fun clear() = Unit
    }

    @Test
    fun transportSendsWrappedAarqAndUnwrapsAare() = runBlocking {
        val link = ScriptedLink(guruxAareFrame)
        val t = WrapperTransport(link, ClientProfile.PUBLIC.clientPort)
        val r = Association.parseResponse(t.request(Association.publicRequest()))
        assertTrue(r.accepted)
        assertEquals(Conformance.GET, r.negotiatedConformance)
        assertEquals(0x02e2, r.serverMaxReceivePdu)
        assertArrayEquals(guruxAarqFrame, link.sent.single())
    }

    @Test
    fun transportIgnoresFramesForOtherClients() = runBlocking {
        val other = hex("0001 0001 0020 0001 aa")
        val ours = hex("0001 0001 0010 0005 c401c10011")
        val t = WrapperTransport(ScriptedLink(other + ours), ClientProfile.PUBLIC.clientPort)
        assertArrayEquals(hex("c401c10011"), t.request(Get.request(PublicObjects.SERIAL_NUMBER)))
    }

    @Test
    fun transportReportsSilenceAndTruncation() = runBlocking {
        val silent = WrapperTransport(ScriptedLink(), ClientProfile.PUBLIC.clientPort)
        assertEquals("No response from meter", runCatching { silent.request(hex("c0")) }.exceptionOrNull()?.message)
        val cut = WrapperTransport(ScriptedLink(hex("0001 0001 0010 0010 c4")), ClientProfile.PUBLIC.clientPort)
        assertEquals("Incomplete response (9 bytes)", runCatching { cut.request(hex("c0")) }.exceptionOrNull()?.message)
    }

    @Test
    fun obisRoundTrip() {
        assertEquals("0.0.40.0.1.255", ObisCode.parse("0.0.40.0.1.255").toString())
        assertArrayEquals(hex("0000280001ff"), ObisCode.parse("0.0.40.0.1.255").bytes)
    }
}
