package com.example.meterlink.data.model

data class MeterInfo(
    val physical: String = "",
    val device: String = "",
    val account: String = "",
    val key: String = "",
    val logical: String = "",
    val rank: String = "",
    val access: String = "",
    val status: String = ""
) {
    companion object {
        fun fromString(str: String): MeterInfo? {
            val parts = str.split(",")
            if (parts.size != 8) return null
            return MeterInfo(
                physical = parts[0],
                device = parts[1],
                account = parts[2],
                key = parts[3],
                logical = parts[4],
                rank = parts[5],
                access = parts[6],
                status = parts[7]
            )
        }
    }

    fun toStorageString(): String = "$physical,$device,$account,$key,$logical,$rank,$access,$status"
}

enum class UserRank(val value: Int) {
    SUPER(0),
    ADMIN(1),
    POWER(2),
    READER(3),
    PUBLIC(4)
}