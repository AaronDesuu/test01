package com.example.meterlinkind.data.protocol

/**
 * A value being edited field by field. A structure or array (a script table,
 * an action schedule's times) becomes a [Group] of its elements, each scalar
 * a [Leaf] with its own [ValueEditor]. Immutable: every change returns a new tree.
 */
sealed class ValueTree {
    /** One scalar, edited as [text]. */
    class Leaf(val editor: ValueEditor, val text: String) : ValueTree()

    /** A value with nothing to edit (null-data). */
    class Fixed(val value: DlmsData) : ValueTree()

    /**
     * A structure or an array. An array can grow by a copy of [template]
     * (its first element as read) and shrink to nothing.
     */
    class Group(val array: Boolean, val items: List<ValueTree>, val template: ValueTree?) : ValueTree()

    /** The value, or the first field that isn't one, named by its place. */
    fun build(): Result<DlmsData> = runCatching { buildAt("") }

    private fun buildAt(place: String): DlmsData = when (this) {
        is Fixed -> value
        is Leaf -> editor.parse(text).getOrElse { throw IllegalArgumentException(if (place.isEmpty()) it.message else "$place: ${it.message}") }
        is Group -> {
            val values = items.mapIndexed { i, item -> item.buildAt(childName(place, i)) }
            if (array) DlmsData.Array(values) else DlmsData.Structure(values)
        }
    }

    private fun childName(place: String, index: Int): String {
        val own = if ((this as Group).array) "Item ${index + 1}" else "Field ${index + 1}"
        return if (place.isEmpty()) own else "$place › $own"
    }

    /** The tree with the node at [path] (child indexes from here) replaced by [change] of it. */
    fun update(path: List<Int>, change: (ValueTree) -> ValueTree): ValueTree {
        if (path.isEmpty()) return change(this)
        val group = this as Group
        val i = path.first()
        val items = group.items.toMutableList()
        items[i] = items[i].update(path.drop(1), change)
        return Group(group.array, items, group.template)
    }

    companion object {
        /**
         * The tree for [value]. Only the top-level value takes [scaler]; inside
         * a structure a 6-byte octet-string is edited as an OBIS code.
         */
        fun of(value: DlmsData, scaler: ScalerUnit? = null): ValueTree = when (value) {
            is DlmsData.Structure -> Group(false, value.items.map(::element), null)
            is DlmsData.Array -> value.items.map(::element).let { Group(true, it, it.firstOrNull()) }
            DlmsData.Null -> Fixed(value)
            else -> ValueEditor.forValue(value, scaler).let { Leaf(it, it.initial) }
        }

        private fun element(value: DlmsData): ValueTree =
            if (value is DlmsData.OctetString && value.bytes.size == 6) {
                ValueEditor.forObis(value.bytes).let { Leaf(it, it.initial) }
            } else of(value)

        /** True when [value] is a structure or array, edited as a [Group]. */
        fun isGroup(value: DlmsData) = value is DlmsData.Structure || value is DlmsData.Array
    }
}
