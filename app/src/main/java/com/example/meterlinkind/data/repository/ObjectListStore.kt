package com.example.meterlinkind.data.repository

import android.content.Context
import android.util.Log
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.data.protocol.ObisCode
import com.example.meterlinkind.data.protocol.ObjectListEntry
import com.google.gson.Gson
import dagger.hilt.android.qualifiers.ApplicationContext
import java.io.File
import javax.inject.Inject
import javax.inject.Singleton

/** A meter's object list as read from it, with when it was read. */
data class StoredObjectList(val readAtMillis: Long, val objects: List<ObjectListEntry>)

/** One saved object list, as Settings lists them. */
data class SavedMeter(val serial: String, val client: ClientProfile, val readAtMillis: Long, val objectCount: Int)

/**
 * Keeps each meter's object list (current association, attribute 2) on the
 * phone, one JSON file per meter serial and client, so it is read from the
 * meter once rather than on every connection. US and MR may see different
 * objects, hence the client in the key.
 */
@Singleton
class ObjectListStore @Inject constructor(
    @ApplicationContext context: Context
) {
    private val dir = File(context.filesDir, "object_lists")
    private val gson = Gson()

    private class Json(val readAtMillis: Long, val objects: List<JsonEntry>)
    /** [access] is absent in lists saved before access rights were kept. */
    private class JsonEntry(val classId: Int, val version: Int, val obis: String, val access: Map<Int, Int>?)

    fun load(serial: String, client: ClientProfile): StoredObjectList? {
        val file = file(serial, client)
        if (!file.exists()) return null
        return try {
            val json = gson.fromJson(file.readText(), Json::class.java)
            StoredObjectList(json.readAtMillis, json.objects.map { ObjectListEntry(it.classId, it.version, ObisCode.parse(it.obis), it.access) })
        } catch (e: Exception) {
            Log.w("ObjectListStore", "Ignoring unreadable ${file.name}: ${e.message}")
            null
        }
    }

    fun save(serial: String, client: ClientProfile, list: StoredObjectList) {
        dir.mkdirs()
        val json = Json(list.readAtMillis, list.objects.map { JsonEntry(it.classId, it.version, it.obis.toString(), it.attributeAccess) })
        file(serial, client).writeText(gson.toJson(json))
    }

    /** Every saved list, newest first. */
    fun list(): List<SavedMeter> =
        dir.listFiles { f -> f.name.endsWith(".json") }.orEmpty().mapNotNull { file ->
            val base = file.name.removeSuffix(".json")
            val client = ClientProfile.entries.firstOrNull { base.endsWith("_" + it.name.lowercase()) } ?: return@mapNotNull null
            val serial = base.removeSuffix("_" + client.name.lowercase())
            val stored = load(serial, client) ?: return@mapNotNull null
            SavedMeter(serial, client, stored.readAtMillis, stored.objects.size)
        }.sortedByDescending { it.readAtMillis }

    fun delete(serial: String, client: ClientProfile) {
        file(serial, client).delete()
    }

    fun deleteAll() {
        dir.listFiles().orEmpty().forEach { it.delete() }
    }

    private fun file(serial: String, client: ClientProfile): File {
        // Serials are ASCII from the meter; keep only file-name-safe characters.
        val safe = serial.filter { it.isLetterOrDigit() || it == '-' || it == '_' }.ifEmpty { "unknown" }
        return File(dir, "${safe}_${client.name.lowercase()}.json")
    }
}
