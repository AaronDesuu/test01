package com.example.meterlinkind.data.model

import com.example.meterlinkind.data.protocol.ClientProfile
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.util.concurrent.TimeUnit

class UserSessionTest {
    private val lastActive = 1_000_000_000_000L
    private val session = UserSession(ClientProfile.UTILITY_SETTINGS, lastActive)

    @Test
    fun expiresAfterThreeDays() {
        assertFalse(session.isExpired(lastActive + TimeUnit.HOURS.toMillis(1)))
        assertFalse(session.isExpired(lastActive + TimeUnit.DAYS.toMillis(3) - 1))
        assertTrue(session.isExpired(lastActive + TimeUnit.DAYS.toMillis(3)))
    }
}
