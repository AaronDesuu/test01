package com.example.meterlinkind.data.model

import com.example.meterlinkind.data.protocol.ClientProfile
import java.util.concurrent.TimeUnit

/** A logged-in app user: their DLMS client and when they last used the app. */
data class UserSession(val client: ClientProfile, val lastActiveMillis: Long) {
    /** True once the app has gone unused for [TIMEOUT_MILLIS]; the user must log in again. */
    fun isExpired(nowMillis: Long): Boolean = nowMillis - lastActiveMillis >= TIMEOUT_MILLIS

    companion object {
        /** Logged out after 3 days without opening the app. */
        val TIMEOUT_MILLIS: Long = TimeUnit.DAYS.toMillis(3)
    }
}
