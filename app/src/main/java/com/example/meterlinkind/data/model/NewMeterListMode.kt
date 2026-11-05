package com.example.meterlinkind.data.model

/**
 * What to do when a meter without a saved object list connects and another
 * meter's list (same client) is on the phone. All FT5AB meters normally serve
 * the same objects.
 */
enum class NewMeterListMode(val label: String) {
    ASK("Ask"),
    REUSE("Always reuse"),
    READ("Always read"),
}
