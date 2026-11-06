package com.example.meterlinkind.data.protocol

import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ValueTreeTest {
    private fun hex(s: String) = ValueEditor.hexToBytes(s)

    // An action schedule's execution_time: array of {time, date}, plus a script reference with an OBIS code.
    private val schedule = DlmsData.Array(
        listOf(
            DlmsData.Structure(listOf(DlmsData.OctetString(hex("0C000000")), DlmsData.OctetString(hex("FFFFFFFFFF")))),
        )
    )

    @Test
    fun unchangedTreeBuildsTheSameValue() {
        val script = DlmsData.Structure(listOf(DlmsData.OctetString(hex("00000A0064FF")), DlmsData.Integer(Axdr.TAG_UINT16, 1)))
        for (v in listOf(schedule, script, DlmsData.Integer(Axdr.TAG_UINT8, 7), DlmsData.Bool(true))) {
            assertArrayEquals(Axdr.encode(v), Axdr.encode(ValueTree.of(v).build().getOrThrow()))
        }
    }

    @Test
    fun sixByteOctetStringsInsideAreObisCodes() {
        val tree = ValueTree.of(DlmsData.Structure(listOf(DlmsData.OctetString(hex("00000A0064FF")), DlmsData.Integer(Axdr.TAG_UINT16, 1))))
        val obis = ((tree as ValueTree.Group).items[0] as ValueTree.Leaf)
        assertEquals("0.0.10.0.100.255", obis.text)
        val changed = tree.update(listOf(0)) { ValueTree.Leaf(obis.editor, "0.0.10.0.101.255") }
        val built = changed.build().getOrThrow() as DlmsData.Structure
        assertArrayEquals(hex("00000A0065FF"), (built.items[0] as DlmsData.OctetString).bytes)
    }

    @Test
    fun editingANestedFieldChangesOnlyThatField() {
        val tree = ValueTree.of(schedule)
        val time = ((tree as ValueTree.Group).items[0] as ValueTree.Group).items[0] as ValueTree.Leaf
        val changed = tree.update(listOf(0, 0)) { ValueTree.Leaf(time.editor, "0D000000") }
        assertArrayEquals(Axdr.encode(DlmsData.Array(listOf(DlmsData.Structure(listOf(
            DlmsData.OctetString(hex("0D000000")), DlmsData.OctetString(hex("FFFFFFFFFF"))
        ))))), Axdr.encode(changed.build().getOrThrow()))
    }

    @Test
    fun arraysGrowFromTheirFirstItem() {
        val tree = ValueTree.of(schedule) as ValueTree.Group
        val grown = ValueTree.Group(true, tree.items + tree.template!!, tree.template)
        assertEquals(2, (grown.build().getOrThrow() as DlmsData.Array).items.size)
    }

    @Test
    fun errorsNameTheField() {
        val tree = ValueTree.of(DlmsData.Structure(listOf(DlmsData.Integer(Axdr.TAG_INT8, -2), DlmsData.Enum(35))))
        val unit = (tree as ValueTree.Group).items[1] as ValueTree.Leaf
        val bad = tree.update(listOf(1)) { ValueTree.Leaf(unit.editor, "300") }
        val message = bad.build().exceptionOrNull()?.message.orEmpty()
        assertTrue(message, message.startsWith("Field 2:"))
    }
}
