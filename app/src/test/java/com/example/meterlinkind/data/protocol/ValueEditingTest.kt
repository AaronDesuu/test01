package com.example.meterlinkind.data.protocol

import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ValueEditingTest {
    private fun hex(s: String) = ValueEditor.hexToBytes(s)

    @Test
    fun encodeReadsBackEveryType() {
        val values = listOf(
            DlmsData.Null,
            DlmsData.Bool(true),
            DlmsData.Integer(Axdr.TAG_INT8, -5),
            DlmsData.Integer(Axdr.TAG_UINT16, 23912),
            DlmsData.Integer(Axdr.TAG_INT32, -100000),
            DlmsData.Integer(Axdr.TAG_UINT32, 4_000_000_000),
            DlmsData.Integer(Axdr.TAG_INT64, -1),
            DlmsData.Enum(27),
            DlmsData.Float(239.5, singlePrecision = true),
            DlmsData.Float(0.1),
            DlmsData.VisibleString("FT5AB"),
            DlmsData.Utf8String("meter"),
            DlmsData.Structure(listOf(DlmsData.Integer(Axdr.TAG_INT8, -2), DlmsData.Enum(35))),
            DlmsData.Array(listOf(DlmsData.Integer(Axdr.TAG_UINT8, 1), DlmsData.Integer(Axdr.TAG_UINT8, 2))),
        )
        for (v in values) assertEquals(v.toString(), v.display(), Axdr.decodeWhole(Axdr.encode(v)).display())
    }

    @Test
    fun encodeMatchesTheWire() {
        assertArrayEquals(hex("12 5D68"), Axdr.encode(DlmsData.Integer(Axdr.TAG_UINT16, 23912)))
        assertArrayEquals(hex("02 02 0F FE 16 23"), Axdr.encode(DlmsData.Structure(listOf(DlmsData.Integer(Axdr.TAG_INT8, -2), DlmsData.Enum(35)))))
        assertArrayEquals(hex("09 03 414243"), Axdr.encode(DlmsData.OctetString("ABC".toByteArray())))
        assertArrayEquals(hex("04 0A C040"), Axdr.encode(DlmsData.BitString(10, hex("C040"))))
    }

    @Test
    fun registerValueIsEditedInItsUnit() {
        val editor = ValueEditor.forValue(DlmsData.Integer(Axdr.TAG_UINT16, 23912), ScalerUnit(-2, 35))
        assertEquals("239.12", editor.initial)
        assertEquals(DlmsData.Integer(Axdr.TAG_UINT16, 24000), editor.parse("240").getOrThrow())
        assertTrue(editor.parse("240.005").isFailure) // not a multiple of 0.01
        assertTrue(editor.parse("700").isFailure) // over long-unsigned
    }

    @Test
    fun integersStayInTheirRange() {
        val editor = ValueEditor.forValue(DlmsData.Integer(Axdr.TAG_UINT8, 10))
        assertEquals(DlmsData.Integer(Axdr.TAG_UINT8, 11), editor.parse("11").getOrThrow())
        assertTrue(editor.parse("256").isFailure)
        assertTrue(editor.parse("-1").isFailure)
        assertTrue(editor.parse("ten").isFailure)
    }

    @Test
    fun octetStringsAsTextHexOrDateTime() {
        val text = ValueEditor.forValue(DlmsData.OctetString("CLIENT00".toByteArray()))
        assertEquals(ValueEditor.Kind.TEXT, text.kind)
        assertArrayEquals("CLIENT01".toByteArray(), (text.parse("CLIENT01").getOrThrow() as DlmsData.OctetString).bytes)

        val bytes = ValueEditor.forValue(DlmsData.OctetString(hex("00FF10")))
        assertEquals(ValueEditor.Kind.HEX, bytes.kind)
        assertArrayEquals(hex("0102"), (bytes.parse("01 02").getOrThrow() as DlmsData.OctetString).bytes)
        assertTrue(bytes.parse("123").isFailure)

        // 2026-11-05 10:20:30, deviation -330 (0xFEB6), status 0x80, kept on write.
        val clock = ValueEditor.forValue(DlmsData.OctetString(hex("07EA0B0504 0A141E FF FEB6 80")))
        assertEquals(ValueEditor.Kind.DATE_TIME, clock.kind)
        assertEquals("2026-11-05 10:20:30", clock.initial)
        assertArrayEquals(
            hex("07EA0B0605 0B0000 FF FEB6 80"),
            (clock.parse("2026-11-06 11:00:00").getOrThrow() as DlmsData.OctetString).bytes
        )
        assertTrue(clock.parse("6 Nov 2026").isFailure)
    }

    @Test
    fun structuresAreEditedAsEncodedHex() {
        val editor = ValueEditor.forValue(DlmsData.Structure(listOf(DlmsData.Integer(Axdr.TAG_INT8, -2), DlmsData.Enum(35))))
        assertEquals("0202 0FFE 1623".replace(" ", ""), editor.initial)
        assertEquals("{-1, 35}", editor.parse("02020FFF1623").getOrThrow().display())
        assertTrue(editor.parse("02020FFF16").isFailure)   // cut short
        assertTrue(editor.parse("02020FFF162300").isFailure) // trailing byte
    }

    @Test
    fun bitsAndBooleans() {
        val bits = ValueEditor.forValue(DlmsData.BitString(10, hex("C040")))
        assertEquals("1100000001", bits.initial)
        assertArrayEquals(hex("A0"), (bits.parse("101").getOrThrow() as DlmsData.BitString).bytes)
        val flag = ValueEditor.forValue(DlmsData.Bool(false))
        assertEquals(DlmsData.Bool(true), flag.parse("true").getOrThrow())
    }
}
