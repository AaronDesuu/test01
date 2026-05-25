package com.example.meterlink.data.export

enum class OperationType(val slug: String) {
    MEASURE("measure"),
    STATE("state"),
    BILLING("billing"),
    EVENT("event"),
    LOAD("load"),
    AMPERE("ampere"),
}

object CsvExporter {

    fun export(
        operation: OperationType,
        deviceName: String?,
        serialNumber: String?,
        lines: List<String>,
    ): String {
        val sb = StringBuilder()
        val readAt = lines.firstOrNull().orEmpty()
        sb.appendComment("device", deviceName.orEmpty())
        sb.appendComment("serial", serialNumber.orEmpty())
        sb.appendComment("operation", operation.slug)
        sb.appendComment("read_at", readAt)

        val body = lines.drop(1)
        when (operation) {
            OperationType.MEASURE, OperationType.STATE -> emitKeyValue(sb, body)
            OperationType.BILLING -> emitBilling(sb, body)
            OperationType.LOAD -> emitLoad(sb, body)
            OperationType.EVENT -> emitEvent(sb, body)
            OperationType.AMPERE -> emitAmpere(sb, body)
        }
        return sb.toString()
    }

    // --- snapshot / state ---------------------------------------------------

    private fun emitKeyValue(sb: StringBuilder, body: List<String>) {
        sb.appendRow("field", "value")
        for (line in body) {
            if (line.isBlank()) continue
            val sep = line.indexOf(':')
            if (sep >= 0) {
                sb.appendRow(line.substring(0, sep).trim(), line.substring(sep + 1).trim())
            } else {
                sb.appendRow(line.trim(), "")
            }
        }
    }

    // --- billing ------------------------------------------------------------

    // 7 lines per record (DLMS.java:1736-1766):
    //   <timestamp>
    //   IMP: x.xxx [kWh], EXP: x.xxx [kWh]
    //   ABS: x.xxx [kWh], NET: x.xxx [kWh]
    //   Max Imp: x.xxx [kW], Exp: x.xxx [kW]
    //   Volt0 Min: x.xx [V]
    //   Alert1 Dsc: <bitstr>
    //   Alert2 Dsc: <bitstr>
    private val BILLING_IMPEXP =
        Regex("""IMP:\s*([0-9.\-]+)\s*\[kWh\],\s*EXP:\s*([0-9.\-]+)\s*\[kWh\]""")
    private val BILLING_ABSNET =
        Regex("""ABS:\s*([0-9.\-]+)\s*\[kWh\],\s*NET:\s*([0-9.\-]+)\s*\[kWh\]""")
    private val BILLING_MAX =
        Regex("""Max Imp:\s*([0-9.\-]+)\s*\[kW\],\s*Exp:\s*([0-9.\-]+)\s*\[kW\]""")
    private val BILLING_VOLT = Regex("""Volt0 Min:\s*([0-9.\-]+)\s*\[V\]""")
    private val BILLING_ALERT1 = Regex("""Alert1 Dsc:\s*(.*)""")
    private val BILLING_ALERT2 = Regex("""Alert2 Dsc:\s*(.*)""")

    private fun emitBilling(sb: StringBuilder, body: List<String>) {
        sb.appendRow(
            "timestamp", "imp_kwh", "exp_kwh", "abs_kwh", "net_kwh",
            "max_imp_kw", "max_exp_kw", "volt0_min_v", "alert1", "alert2"
        )
        var i = 0
        while (i + 6 < body.size) {
            val ts = body[i]
            val impExp = BILLING_IMPEXP.find(body[i + 1])
            val absNet = BILLING_ABSNET.find(body[i + 2])
            val max = BILLING_MAX.find(body[i + 3])
            val volt = BILLING_VOLT.find(body[i + 4])
            val alert1 = BILLING_ALERT1.find(body[i + 5])
            val alert2 = BILLING_ALERT2.find(body[i + 6])
            sb.appendRow(
                ts,
                impExp?.groupValues?.get(1).orEmpty(),
                impExp?.groupValues?.get(2).orEmpty(),
                absNet?.groupValues?.get(1).orEmpty(),
                absNet?.groupValues?.get(2).orEmpty(),
                max?.groupValues?.get(1).orEmpty(),
                max?.groupValues?.get(2).orEmpty(),
                volt?.groupValues?.get(1).orEmpty(),
                alert1?.groupValues?.get(1).orEmpty(),
                alert2?.groupValues?.get(1).orEmpty(),
            )
            i += 7
        }
    }

    // --- load profile -------------------------------------------------------

    // 4 lines per record (DLMS.java:1768-1775):
    //   <timestamp>
    //   Status   : <bitstr>
    //   Volt0 Ave: x.xx [V]
    //   Block Imp: x.xxx [kW], Exp: x.xxx [kW]
    private val LOAD_STATUS = Regex("""Status\s*:\s*(.*)""")
    private val LOAD_VOLT = Regex("""Volt0 Ave:\s*([0-9.\-]+)\s*\[V\]""")
    private val LOAD_BLOCK =
        Regex("""Block Imp:\s*([0-9.\-]+)\s*\[kW\],\s*Exp:\s*([0-9.\-]+)\s*\[kW\]""")

    private fun emitLoad(sb: StringBuilder, body: List<String>) {
        sb.appendRow("timestamp", "status", "volt0_ave_v", "block_imp_kw", "block_exp_kw")
        var i = 0
        while (i + 3 < body.size) {
            val ts = body[i]
            val status = LOAD_STATUS.find(body[i + 1])
            val volt = LOAD_VOLT.find(body[i + 2])
            val block = LOAD_BLOCK.find(body[i + 3])
            sb.appendRow(
                ts,
                status?.groupValues?.get(1).orEmpty(),
                volt?.groupValues?.get(1).orEmpty(),
                block?.groupValues?.get(1).orEmpty(),
                block?.groupValues?.get(2).orEmpty(),
            )
            i += 4
        }
    }

    // --- event / power quality ---------------------------------------------

    // 3 lines per record (DLMS.java:1782-1789):
    //   <timestamp>
    //   Event: <PQCODE>
    //   Volt0: x.xx [V]
    private val EVENT_EVENT = Regex("""Event:\s*(.*)""")
    private val EVENT_VOLT = Regex("""Volt0:\s*([0-9.\-]+)\s*\[V\]""")

    private fun emitEvent(sb: StringBuilder, body: List<String>) {
        sb.appendRow("timestamp", "event_code", "volt0_v")
        var i = 0
        while (i + 2 < body.size) {
            val ts = body[i]
            val ev = EVENT_EVENT.find(body[i + 1])
            val volt = EVENT_VOLT.find(body[i + 2])
            sb.appendRow(
                ts,
                ev?.groupValues?.get(1).orEmpty(),
                volt?.groupValues?.get(1).orEmpty(),
            )
            i += 3
        }
    }

    // --- ampere -------------------------------------------------------------

    // 2 lines per record (DLMS.java:1776-1781):
    //   <timestamp>
    //   Current L1 Ave: x.xx [A]
    private val AMPERE_CURRENT = Regex("""Current L1 Ave:\s*([0-9.\-]+)\s*\[A\]""")

    private fun emitAmpere(sb: StringBuilder, body: List<String>) {
        sb.appendRow("timestamp", "current_l1_ave_a")
        var i = 0
        while (i + 1 < body.size) {
            val ts = body[i]
            val cur = AMPERE_CURRENT.find(body[i + 1])
            sb.appendRow(ts, cur?.groupValues?.get(1).orEmpty())
            i += 2
        }
    }

    // --- CSV plumbing -------------------------------------------------------

    private fun StringBuilder.appendComment(key: String, value: String) {
        // Excel/Sheets read these as a normal row; pandas with comment='#' skips them.
        append("# ").append(escape(key)).append(',').append(escape(value)).append("\r\n")
    }

    private fun StringBuilder.appendRow(vararg cells: String) {
        for ((idx, cell) in cells.withIndex()) {
            if (idx > 0) append(',')
            append(escape(cell))
        }
        append("\r\n")
    }

    private fun escape(value: String): String {
        // RFC 4180 minimal escaping
        val needsQuoting = value.any { it == ',' || it == '"' || it == '\n' || it == '\r' }
        if (!needsQuoting) return value
        return "\"" + value.replace("\"", "\"\"") + "\""
    }
}
