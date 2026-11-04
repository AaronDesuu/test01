package com.example.meterlinkind.data

import com.example.meterlinkind.data.model.ClientSecurity
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class ClientSecurityTest {
    @Test
    fun asciiAndHexAreTheSameBytes() {
        assertEquals("30303030303030303030303030303030", ClientSecurity.asciiToHex("0000000000000000"))
        assertEquals("ABCDEFGHIJKLMNOP", ClientSecurity.hexToAsciiOrNull("4142434445464748494A4B4C4D4E4F50"))
        assertEquals(ClientSecurity.DEFAULT, ClientSecurity.asciiToHex("0000000000000000"))
    }

    @Test
    fun hexThatIsNotPrintableHasNoAscii() {
        assertNull(ClientSecurity.hexToAsciiOrNull("00112233445566778899AABBCCDDEEFF"))
        assertNull(ClientSecurity.hexToAsciiOrNull("303"))
    }

    @Test
    fun storedValuesFromBeforeHexEntryStillLoad() {
        assertEquals(ClientSecurity.DEFAULT, ClientSecurity.fromStored("0000000000000000"))
        assertEquals("00112233445566778899AABBCCDDEEFF", ClientSecurity.fromStored("00112233445566778899aabbccddeeff"))
        assertNull(ClientSecurity.fromStored("short"))
    }

    @Test
    fun keysBecomeTheirBytes() {
        val security = ClientSecurity(blockCipherKey = "000102030405060708090A0B0C0D0E0F")
        assertTrue(security.isValid)
        assertArrayEquals(ByteArray(16) { it.toByte() }, security.cipherKeys.blockCipherKey)
        assertArrayEquals("0000000000000000".toByteArray(), security.passwordBytes)
        assertFalse(ClientSecurity(authPassword = "0000000000000000").isValid)
    }
}
