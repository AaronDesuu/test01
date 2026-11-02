package com.example.meterlinkind.data.protocol

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test
import java.time.LocalDateTime

class ObjectsTest {

    private fun hex(s: String): ByteArray =
        s.replace(" ", "").chunked(2).map { it.toInt(16).toByte() }.toByteArray()

    // --- Registry ------------------------------------------------------------

    @Test
    fun registryMatchesFirmwareTable() {
        assertEquals(457, ObjectRegistry.all.size)
        assertEquals(118, ObjectRegistry.ofClass(InterfaceClass.DATA).size)
        assertEquals(19, ObjectRegistry.ofClass(InterfaceClass.PROFILE_GENERIC).size)
        assertEquals(2, ObjectRegistry.ofClass(InterfaceClass.CLOCK).size)
        val blockLoad = ObjectRegistry.find(ObisCode.parse("1.0.99.1.0.255"))!!
        assertEquals("Block load profile", blockLoad.name)
        assertEquals(408, blockLoad.index)
        assertEquals(1, blockLoad.version)
        assertEquals("Meter serial number", ObjectRegistry.find(PublicObjects.SERIAL_NUMBER.obis)!!.name)
    }

    // --- GET with selective access, SET --------------------------------------

    @Test
    fun getWithEntryDescriptor() {
        val profile = ObjectRegistry.find(ObisCode.parse("1.0.99.1.0.255"))!!
        assertArrayEquals(
            hex("C0 01 C1 0007 0100630100FF 02 01 02 0204 06 0000005B 06 00000064 12 0001 12 0000"),
            Get.request(profile.attribute(2), SelectiveAccess.byEntry(91, 100))
        )
    }

    @Test
    fun getWithRangeDescriptor() {
        val clockColumn = CaptureObject(8, ObisCode.parse("0.0.1.0.0.255"), 2, 0)
        val from = LocalDateTime.of(2026, 11, 2, 0, 0)
        val to = LocalDateTime.of(2026, 11, 2, 15, 34)
        assertArrayEquals(
            hex(
                "01 02 04" +
                    "02 04 12 0008 09 06 0000010000FF 0F 02 12 0000" + // restricting object: clock column
                    "09 0C 07EA 0B 02 01 00 00 00 FF 8000 00" +       // from 2026-11-02 00:00:00 (Monday)
                    "09 0C 07EA 0B 02 01 0F 22 00 FF 8000 00" +       // to 15:34:00
                    "01 00"                                          // all columns
            ),
            SelectiveAccess.byRange(clockColumn, from, to)
        )
    }

    @Test
    fun float32DisplaysAtSinglePrecision() {
        // 239.73 as the meter's block voltage column sends it
        assertEquals("239.73", Axdr.decode(hex("17 436FBAE1")).display())
    }

    @Test
    fun lastDaysStartsAtMidnight() {
        val now = LocalDateTime.of(2026, 11, 2, 15, 34, 10)
        assertEquals(
            LocalDateTime.of(2026, 11, 2, 0, 0) to now,
            com.example.meterlinkind.presentation.viewmodel.BufferSelection.LastDays(0).dateRange(now)
        )
        assertEquals(
            LocalDateTime.of(2026, 10, 31, 0, 0) to now,
            com.example.meterlinkind.presentation.viewmodel.BufferSelection.LastDays(2).dateRange(now)
        )
    }

    @Test
    fun setRequestAndResponse() {
        val clock = ObjectRegistry.find(ObisCode.parse("0.0.1.0.0.255"))!!
        val time = ClockValue.encode(LocalDateTime.of(2026, 11, 2, 14, 35, 10), deviation = 0xFEB6, status = 0)
        // 2026-11-02 is a Monday (1); hundredths not specified; deviation -330 min.
        assertArrayEquals(hex("09 0C 07EA 0B 02 01 0E 23 0A FF FEB6 00"), time)
        assertArrayEquals(hex("C1 01 C1 0008 0000010000FF 02 00") + time, CosemSet.request(clock.attribute(2), time))
        assertNull(CosemSet.parseResponse(hex("C5 01 C1 00")))
        assertEquals("read/write denied (3)", CosemSet.parseResponse(hex("C5 01 C1 03")))
        assertEquals(0xFEB6, ClockValue.deviationOf(time.copyOfRange(2, 14)))
    }

    // --- Structures ------------------------------------------------------------

    @Test
    fun parsesCaptureObjects() {
        val data = Axdr.decode(
            hex(
                "01 02" +
                    "02 04 12 0008 09 06 0000010000FF 0F 02 12 0000" +
                    "02 04 12 0003 09 06 0100200700FF 0F 02 12 0000"
            )
        )
        val columns = CosemStructures.captureObjects(data)
        assertEquals(2, columns.size)
        assertEquals(CaptureObject(8, ObisCode.parse("0.0.1.0.0.255"), 2, 0), columns[0])
        assertEquals("Date time", columns[0].label)
        assertEquals(3, columns[1].classId)
    }

    @Test
    fun parsesObjectListWithAccessRights() {
        // As the FT5AB sends it: {[{attr, mode, null}...], []} per object.
        val data = Axdr.decode(
            hex(
                "01 02" +
                    "02 04 12 0008 11 00 09 06 0000010000FF" +
                    "  02 02 01 02 02 03 0F 01 16 01 00 02 03 0F 02 16 03 00 01 00" +
                    "02 04 12 0007 11 01 09 06 0100630100FF 02 02 01 00 01 00"
            )
        )
        val list = CosemStructures.objectList(data)
        assertEquals(ObjectListEntry(8, 0, ObisCode.parse("0.0.1.0.0.255"), mapOf(1 to 1, 2 to 3)), list[0])
        assertEquals(ObjectListEntry(7, 1, ObisCode.parse("1.0.99.1.0.255"), emptyMap()), list[1])

        val clock = CosemObject(1, 8, 0, list[0].obis, "Clock", list[0].attributeAccess)
        assertTrue(clock.canRead(1) && !clock.canWrite(1))
        assertTrue(clock.canRead(2) && clock.canWrite(2))
        assertFalse(clock.canRead(3)) // not listed: no access
        assertTrue(CosemObject(1, 8, 0, list[0].obis, "Clock").canRead(3)) // unknown: allowed
        assertEquals("R/W", AccessMode.label(3))
    }

    // --- General Block Transfer ----------------------------------------------

    @Test
    fun reassemblesStreamedGbt() {
        val apdu = hex("C4 01 C1 00 09 03 41 42 43")
        val b1 = GbtBlock.parse(hex("E0 40 0001 0000 04") + apdu.copyOfRange(0, 4))
        val b2 = GbtBlock.parse(hex("E0 C0 0002 0000 05") + apdu.copyOfRange(4, 9))
        assertTrue(b1.streaming && !b1.last)
        assertTrue(b2.last)
        val assembler = GbtAssembler()
        assertNull(assembler.add(b1))
        assertArrayEquals(apdu, assembler.add(b2))
        assertArrayEquals(hex("E0 01 0001 0003 00"), GbtBlock.ack(1, 3))
    }

    @Test
    fun deciphersGeneralGloCiphering() {
        val keys = CipherKeys(ByteArray(16) { 0x30 }, ByteArray(16) { 0x30 })
        val meterTitle = hex("464549000000022C")
        val plain = hex("C4 01 C1 00 09 03 41 42 43")
        val payload = SecuritySuite0.encrypt(0x30, meterTitle, 9, keys, plain)
        val general = hex("DB 08") + meterTitle + Axdr.encodeLength(payload.size) + payload
        // The title comes with the APDU, so the channel needn't know it yet.
        val channel = SecureChannel("CLIENT00".toByteArray(), keys, 0x30) { 1 }
        assertArrayEquals(plain, channel.unprotect(general))
    }

    @Test
    fun transportKeepsSecondFrameOfAChunk() = runBlocking {
        val first = hex("0001 0001 0010 0003 E0 40 01")
        val second = hex("0001 0001 0010 0003 E0 C0 02")
        val link = object : ByteLink {
            var sent = false
            override suspend fun send(data: ByteArray) = true.also { sent = true }
            override suspend fun receive(timeoutMs: Long): ByteArray? =
                if (sent) (first + second).also { sent = false } else null
            override fun clear() = Unit
        }
        val transport = WrapperTransport(link, 0x10)
        assertArrayEquals(hex("E0 40 01"), transport.request(hex("C0")))
        assertArrayEquals(hex("E0 C0 02"), transport.receive())
    }
}
