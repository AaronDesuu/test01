package com.example.meterlinkind.data.protocol

import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class SecurityTest {

    private fun hex(s: String): ByteArray =
        s.replace(" ", "").chunked(2).map { it.toInt(16).toByte() }.toByteArray()

    // DLMS UA Green Book example (also in the Gurux docs): glo-get-request, SC 0x30.
    private val keys = CipherKeys(hex("000102030405060708090A0B0C0D0E0F"), hex("D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"))
    private val systemTitle = hex("4D4D4D0000BC614E")
    private val getRequest = hex("C0 01 00 00 08 00 00 01 00 00 FF 02 00")
    private val gloGetRequest = hex(
        "C8 1E 30 01234567" +
            "411312FF935A47566827C467BC" + // ciphertext
            "7D825C3BE4A77C3FCC056B6B"     // 12-byte tag
    )

    private fun channel(sc: Int, counter: Long = 0x01234567) =
        SecureChannel(systemTitle, keys, sc) { counter }

    @Test
    fun protectMatchesGreenBookExample() {
        assertArrayEquals(gloGetRequest, channel(0x30).protect(getRequest))
    }

    @Test
    fun unprotectReversesIt() {
        val c = channel(0x30).apply { serverSystemTitle = systemTitle }
        assertArrayEquals(getRequest, c.unprotect(gloGetRequest))
    }

    @Test
    fun tamperedTagIsRejected() {
        val c = channel(0x30).apply { serverSystemTitle = systemTitle }
        val bad = gloGetRequest.copyOf().also { it[it.size - 1] = (it[it.size - 1] + 1).toByte() }
        val e = runCatching { c.unprotect(bad) }.exceptionOrNull()
        assertTrue(e is SecurityException)
    }

    @Test
    fun encryptionOnlyHasNoTagAndSameCiphertext() {
        // GCM's ciphertext doesn't depend on the tag, so MR's SC 0x20 frame is the
        // Green Book one without the tag.
        val c = channel(0x20).apply { serverSystemTitle = systemTitle }
        val frame = c.protect(getRequest)
        assertArrayEquals(hex("C8 12 20 01234567 411312FF935A47566827C467BC"), frame)
        assertArrayEquals(getRequest, c.unprotect(frame))
    }

    @Test
    fun plainRepliesPassThrough() {
        val exception = hex("D8 01 05")
        assertArrayEquals(exception, channel(0x30).unprotect(exception))
    }

    @Test
    fun aesEcbMatchesFips197() {
        assertArrayEquals(
            hex("69c4e0d86a7b0430d8cdb78070b4c55a"),
            SecuritySuite0.aesEcb(hex("000102030405060708090a0b0c0d0e0f"), hex("00112233445566778899aabbccddeeff"))
        )
    }

    // --- Association -------------------------------------------------------

    private val zeros = "0000000000000000".toByteArray()
    private val client = "CLIENT00".toByteArray()

    @Test
    fun llsAarqHasAllSixTagsInOrder() {
        val ciphered = channel(0x20, 1).protect(Association.initiateRequest())
        val aarq = Association.cipheredRequest(client, Mechanism.LLS, zeros, ciphered)
        val tags = Ber.parse(Ber.parse(aarq).single().value).map { it.tag }
        assertEquals(listOf(0xA1, 0xA6, 0x8A, 0x8B, 0xAC, 0xBE), tags)
        assertArrayEquals(
            hex("A1 09 06 07 60857405080103") +
                hex("A6 0A 04 08") + client +
                hex("8A 02 07 80") +
                hex("8B 07 60857405080201") +
                hex("AC 12 80 10") + zeros,
            aarq.copyOfRange(2, 2 + 11 + 12 + 4 + 9 + 20)
        )
        // glo-initiate-request: 21 len SC 0x20 FC 00000001 then the 14-byte ciphertext, no tag.
        assertArrayEquals(hex("BE 17 04 15 21 13 20 00000001"), aarq.copyOfRange(58, 69))
    }

    @Test
    fun hlsAarqCarriesChallengeAndMechanism2() {
        val challenge = "ABCDEFGHIJKLMNOP".toByteArray()
        val aarq = Association.cipheredRequest(client, Mechanism.HLS, challenge, hex("21 00"))
        val els = Ber.parse(Ber.parse(aarq).single().value).associateBy { it.tag }
        assertArrayEquals(hex("60857405080202"), els.getValue(0x8B).value)
        assertArrayEquals(hex("80 10") + challenge, els.getValue(0xAC).value)
    }

    @Test
    fun parsesCipheredAare() {
        val meterTitle = hex("4D4D4D0000000001")
        val meter = SecureChannel(meterTitle, keys, 0x30) { 7 }
        // InitiateResponse: version 6, conformance 00 10 1D, max PDU 0x02E2, VAA name 0x0007
        val initiateResponse = hex("08 00 06 5F1F04 00 00101D 02E2 0007")
        val info = meter.protect(initiateResponse)
        val aare = Ber.tlv(
            0x61,
            hex("A1 09 06 07 60857405080103") +
                hex("A2 03 02 01 00") +
                hex("A3 05 A1 03 02 01 0E") +
                Ber.tlv(0xA4, Ber.tlv(0x04, meterTitle)) +
                hex("88 02 07 80") +
                hex("89 07 60857405080202") +
                Ber.tlv(0xAA, Ber.tlv(0x80, "PONMLKJIHGFEDCBA".toByteArray())) +
                Ber.tlv(0xBE, Ber.tlv(0x04, info))
        )
        val ours = channel(0x30)
        val r = Association.parseResponse(aare, ours)
        assertTrue(r.accepted)
        assertEquals(14, r.diagnostic) // authentication required: HLS pass 3 follows
        assertEquals(0x00101D, r.negotiatedConformance)
        assertEquals(0x02E2, r.serverMaxReceivePdu)
        assertArrayEquals(meterTitle, ours.serverSystemTitle)
        assertArrayEquals("PONMLKJIHGFEDCBA".toByteArray(), r.serverChallenge)
    }

    @Test
    fun hlsReplyAndVerify() {
        val stoc = "PONMLKJIHGFEDCBA".toByteArray()
        val request = Association.hlsReplyRequest(zeros, stoc)
        assertArrayEquals(
            hex("C3 01 C1 000F 0000280000FF 01 01 09 10") + SecuritySuite0.aesEcb(zeros, stoc),
            request
        )
        val ctos = "ABCDEFGHIJKLMNOP".toByteArray()
        val good = hex("C7 01 C1 00 01 00 09 10") + SecuritySuite0.aesEcb(zeros, ctos)
        assertTrue(Association.hlsVerify(zeros, ctos, Action.parseResponse(good)))
        val wrong = hex("C7 01 C1 00 01 00 09 10") + SecuritySuite0.aesEcb("1111111111111111".toByteArray(), ctos)
        assertFalse(Association.hlsVerify(zeros, ctos, Action.parseResponse(wrong)))
        val refused = Action.parseResponse(hex("C7 01 C1 02 00"))
        assertEquals(ActionResult.Failure(2), refused)
        assertFalse(Association.hlsVerify(zeros, ctos, refused))
    }
}
