package com.example.meterlinkind.presentation.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.meterlinkind.data.model.NewMeterListMode
import com.example.meterlinkind.data.protocol.Axdr
import com.example.meterlinkind.data.protocol.CaptureObject
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.data.protocol.ClockValue
import com.example.meterlinkind.data.protocol.CosemAttribute
import com.example.meterlinkind.data.protocol.CosemObject
import com.example.meterlinkind.data.protocol.CosemStructures
import com.example.meterlinkind.data.protocol.DisconnectControl
import com.example.meterlinkind.data.protocol.DlmsData
import com.example.meterlinkind.data.protocol.InterfaceClass
import com.example.meterlinkind.data.protocol.ObisCode
import com.example.meterlinkind.data.protocol.ObjectRegistry
import com.example.meterlinkind.data.protocol.ScalerUnit
import com.example.meterlinkind.data.protocol.SelectiveAccess
import com.example.meterlinkind.data.repository.AttributeRead
import com.example.meterlinkind.data.repository.BleConnectionState
import com.example.meterlinkind.data.repository.BleRepository
import com.example.meterlinkind.presentation.screens.BufferReadMode
import com.example.meterlinkind.data.repository.DlmsRepository
import com.example.meterlinkind.data.repository.ObjectListStore
import com.example.meterlinkind.data.repository.SavedMeter
import com.example.meterlinkind.data.repository.SettingsRepository
import com.example.meterlinkind.data.repository.StoredObjectList
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import java.time.LocalDateTime
import javax.inject.Inject

/** One attribute's last read. */
data class AttributeState(
    val loading: Boolean = false,
    val value: DlmsData? = null,
    val error: String? = null,
    val progress: String? = null,
    /** When [value] was read (phone time, ms); null if never. */
    val readAt: Long? = null,
)

/** A write from the edit dialog: [key] as in [ObjectBrowserUiState.key]; [error] null once written. */
data class AttributeWrite(val key: String, val busy: Boolean, val error: String? = null)

/** Progress of "Read all values": [done] of [total] objects; [stopping] after Stop until the current read ends. */
data class ValuesReadProgress(val done: Int, val total: Int, val stopping: Boolean = false)

/** Which part of a Profile Generic buffer to read, as Gurux offers it. */
sealed class BufferSelection {
    /** Rows [first]..[first]+[count]-1 (entry_descriptor). */
    data class Entries(val first: Long, val count: Long) : BufferSelection()
    /** From midnight [days] days ago until now; 0 = today (range_descriptor). */
    data class LastDays(val days: Long) : BufferSelection()
    /** Rows whose time lies in [from]..[to] (range_descriptor). */
    data class Range(val from: LocalDateTime, val to: LocalDateTime) : BufferSelection()
    object All : BufferSelection()

    fun dateRange(now: LocalDateTime = LocalDateTime.now().withNano(0)): Pair<LocalDateTime, LocalDateTime> = when (this) {
        is LastDays -> now.toLocalDate().minusDays(days).atStartOfDay() to now
        is Range -> from to to
        else -> throw IllegalStateException("$this has no date range")
    }

    fun describe(): String = when (this) {
        is Entries -> "entries $first-${first + count - 1}"
        is LastDays -> if (days == 0L) "today's entries" else "the last $days days"
        is Range -> "entries from $from to $to"
        All -> "all entries"
    }
}

/** A Profile Generic buffer read: columns from capture_objects, rows from buffer. */
data class ProfileBufferState(
    val loading: Boolean = false,
    val progress: String? = null,
    val error: String? = null,
    val columns: List<CaptureObject>? = null,
    val rows: List<List<DlmsData>>? = null,
    val entriesInUse: Long? = null,
)

data class ObjectBrowserUiState(
    /** Meter whose object list is shown: its serial and our client, the key in [ObjectListStore]. */
    val meterSerial: String? = null,
    val client: ClientProfile? = null,
    /** That meter's object list, from the phone or just read; null if never read. */
    val objectList: StoredObjectList? = null,
    val readingList: Boolean = false,
    val listMessage: String? = null,
    /** Keyed by [key]. */
    val attributes: Map<String, AttributeState> = emptyMap(),
    val buffers: Map<ObisCode, ProfileBufferState> = emptyMap(),
    /** scaler_unit of Registers read so far; kept across reconnects to the same meter. */
    val scalers: Map<ObisCode, ScalerUnit> = emptyMap(),
    val clockMessage: String? = null,
    /** The write in progress or last finished, for the edit dialog. */
    val write: AttributeWrite? = null,
    /** Set while a list's values are being read; null otherwise. */
    val valuesRead: ValuesReadProgress? = null,
    /** Result of the last Disconnect / Reconnect. */
    val relayMessage: String? = null,
    /** Decimal places for floats and scaled values; null = as sent. From Settings. */
    val decimals: Int? = 2,
    /** Mode the buffer reader opens with. From Settings. */
    val bufferReadMode: BufferReadMode = BufferReadMode.LAST_DAYS,
    /** What happens when a meter without a saved list connects. From Settings. */
    val newMeterListMode: NewMeterListMode = NewMeterListMode.ASK,
    /** Another meter's list this meter could use, while it has none of its own. */
    val reuseCandidate: SavedMeter? = null,
    /** True while the new-meter question is on screen. */
    val askReuse: Boolean = false,
) {
    fun attribute(obj: CosemObject, attribute: Int): AttributeState = attributes[key(obj, attribute)] ?: AttributeState()

    companion object {
        fun key(obj: CosemObject, attribute: Int) = "${obj.obis}/$attribute"
    }
}

/**
 * Gurux-style object browser. The object list comes from the meter itself
 * (current association, attribute 2), read once per meter and client and then
 * kept on the phone by [ObjectListStore]. Attribute values are cached until
 * the BLE link drops.
 */
@HiltViewModel
class ObjectBrowserViewModel @Inject constructor(
    private val dlmsRepository: DlmsRepository,
    private val objectListStore: ObjectListStore,
    private val settingsRepository: SettingsRepository,
    bleRepository: BleRepository,
) : ViewModel() {

    private val _uiState = MutableStateFlow(
        ObjectBrowserUiState(
            decimals = settingsRepository.getDecimals(),
            bufferReadMode = savedReadMode(),
            newMeterListMode = savedNewMeterMode(),
        )
    )
    val uiState: StateFlow<ObjectBrowserUiState> = _uiState.asStateFlow()

    init {
        viewModelScope.launch {
            bleRepository.connectionState.collect { state ->
                if (state is BleConnectionState.Disconnected || state is BleConnectionState.Error) {
                    // Values belong to the session; the object list and scalers stay.
                    _uiState.update { it.copy(attributes = emptyMap(), buffers = emptyMap(), clockMessage = null, relayMessage = null) }
                }
            }
        }
    }

    /** Call when the connected meter or the logged-in client changes; loads its stored object list. */
    fun onMeter(serial: String?, client: ClientProfile?) {
        val state = _uiState.value
        if (serial == state.meterSerial && client == state.client) return
        val stored = if (serial != null && client != null) objectListStore.load(serial, client) else null
        // A meter new to this phone: offer, copy or read according to Settings.
        val candidate = if (serial != null && client != null && stored == null) {
            objectListStore.newestOtherList(serial, client)
        } else null
        val mode = state.newMeterListMode
        _uiState.update {
            ObjectBrowserUiState(
                meterSerial = serial, client = client, objectList = stored,
                decimals = it.decimals, bufferReadMode = it.bufferReadMode, newMeterListMode = mode,
                reuseCandidate = candidate, askReuse = candidate != null && mode == NewMeterListMode.ASK,
            )
        }
        if (serial == null || client == null || stored != null) return
        when {
            mode == NewMeterListMode.READ -> readObjectList()
            mode == NewMeterListMode.REUSE && candidate != null -> reuseList()
        }
    }

    /** Copies the offered list to this meter and shows it. */
    fun reuseList() {
        val state = _uiState.value
        val serial = state.meterSerial ?: return
        val candidate = state.reuseCandidate ?: return
        val copy = objectListStore.copy(candidate, serial)
        _uiState.update {
            it.copy(objectList = copy, reuseCandidate = null, askReuse = false,
                listMessage = if (copy == null) "Couldn't copy the object list from ${candidate.serial}" else null)
        }
    }

    /** Closes the new-meter question; [read] reads this meter's own list. */
    fun dismissReuse(read: Boolean) {
        _uiState.update { it.copy(askReuse = false) }
        if (read) readObjectList()
    }

    // --- Settings ---------------------------------------------------------------

    fun setDecimals(decimals: Int?) {
        settingsRepository.setDecimals(decimals)
        _uiState.update { it.copy(decimals = decimals) }
    }

    fun setBufferReadMode(mode: BufferReadMode) {
        settingsRepository.setBufferReadMode(mode.name)
        _uiState.update { it.copy(bufferReadMode = mode) }
    }

    fun setNewMeterListMode(mode: NewMeterListMode) {
        settingsRepository.setNewMeterListMode(mode.name)
        _uiState.update { it.copy(newMeterListMode = mode) }
    }

    private fun savedNewMeterMode(): NewMeterListMode =
        NewMeterListMode.entries.firstOrNull { it.name == settingsRepository.getNewMeterListMode() } ?: NewMeterListMode.ASK

    private fun savedReadMode(): BufferReadMode =
        BufferReadMode.entries.firstOrNull { it.name == settingsRepository.getBufferReadMode() } ?: BufferReadMode.LAST_DAYS

    fun savedMeters(): List<SavedMeter> = objectListStore.list()

    /** Forgets a saved object list; if it's the one on screen, the lists show "read from meter" again. */
    fun deleteSavedMeter(meter: SavedMeter) {
        objectListStore.delete(meter.serial, meter.client)
        _uiState.update {
            if (it.meterSerial == meter.serial && it.client == meter.client) it.copy(objectList = null) else it
        }
    }

    fun deleteAllSavedMeters() {
        objectListStore.deleteAll()
        _uiState.update { it.copy(objectList = null) }
    }

    /** Objects of [classId] in the meter's list, in the meter's order. */
    fun objects(classId: Int): List<CosemObject> = allObjects().filter { it.classId == classId }

    fun find(obis: String): CosemObject? = allObjects().firstOrNull { it.obis.toString() == obis }

    private fun allObjects(): List<CosemObject> =
        _uiState.value.objectList?.objects.orEmpty().mapIndexed { i, entry ->
            CosemObject(
                index = i + 1,
                classId = entry.classId,
                version = entry.version,
                obis = entry.obis,
                name = ObjectRegistry.find(entry.obis)?.name ?: entry.obis.toString(),
                attributeAccess = entry.attributeAccess,
            )
        }

    /** Reads the object list from the meter and keeps it on the phone for this meter and client. */
    fun readObjectList() {
        val state = _uiState.value
        val serial = state.meterSerial ?: return
        val client = state.client ?: return
        if (state.readingList) return
        viewModelScope.launch {
            _uiState.update { it.copy(readingList = true, listMessage = "Reading object list from the meter...") }
            val result = dlmsRepository.readObjectList { p -> _uiState.update { it.copy(listMessage = p) } }
            result.onSuccess { list ->
                val stored = StoredObjectList(System.currentTimeMillis(), list)
                objectListStore.save(serial, client, stored)
                _uiState.update { it.copy(readingList = false, objectList = stored, listMessage = null, reuseCandidate = null) }
            }.onFailure { e ->
                _uiState.update { it.copy(readingList = false, listMessage = "Couldn't read the object list: ${e.message}") }
            }
        }
    }

    fun read(obj: CosemObject, attribute: Int) {
        val key = ObjectBrowserUiState.key(obj, attribute)
        if (_uiState.value.attributes[key]?.loading == true) return
        viewModelScope.launch { readNow(obj, attribute) }
    }

    /**
     * Reads every attribute this client may read, except a Profile Generic
     * buffer, which has its own read.
     */
    fun readAll(obj: CosemObject) {
        viewModelScope.launch {
            for (attribute in 1..InterfaceClass.attributeCount(obj.classId)) {
                if (obj.classId == InterfaceClass.PROFILE_GENERIC && attribute == BUFFER) continue
                if (!obj.canRead(attribute)) continue
                readNow(obj, attribute, withScaler = false)
            }
        }
    }

    private var valuesJob: Job? = null
    @Volatile private var stopValues = false

    /**
     * Reads [listAttributes] of each of [objects] in turn: the value (with a
     * Register's scaler_unit when it isn't known yet) for classes that have
     * one, every other readable attribute otherwise. Objects with nothing to
     * read are skipped. One run at a time; [stopReadingValues] ends it.
     */
    fun readValues(objects: List<CosemObject>) {
        if (valuesJob?.isActive == true) return
        val todo = objects.filter { listAttributes(it).isNotEmpty() }
        if (todo.isEmpty()) return
        stopValues = false
        valuesJob = viewModelScope.launch {
            try {
                for ((i, obj) in todo.withIndex()) {
                    if (stopValues) break
                    _uiState.update { it.copy(valuesRead = ValuesReadProgress(i, todo.size)) }
                    if (obj.classId in VALUE_CLASSES) {
                        readNow(obj, VALUE)
                    } else {
                        for (attribute in listAttributes(obj)) {
                            if (stopValues) break
                            readNow(obj, attribute, withScaler = false)
                        }
                    }
                }
            } finally {
                _uiState.update { it.copy(valuesRead = null) }
            }
        }
    }

    /**
     * Stops [readValues] once the read in progress has finished. It isn't
     * cancelled midway: the meter's reply would then arrive as the answer to
     * the next request.
     */
    fun stopReadingValues() {
        stopValues = true
        _uiState.update { state -> state.copy(valuesRead = state.valuesRead?.copy(stopping = true)) }
    }

    /** [withScaler]: after a Register value, also read its scaler_unit if unknown (Read all reads it anyway). */
    private suspend fun readNow(obj: CosemObject, attribute: Int, withScaler: Boolean = true): DlmsData? {
        val key = ObjectBrowserUiState.key(obj, attribute)
        setAttribute(key) { it.copy(loading = true, error = null, progress = null) }
        val result = dlmsRepository.read(obj.attribute(attribute)) { p -> setAttribute(key) { it.copy(progress = p) } }
        setAttribute(key) {
            when (result) {
                is AttributeRead.Value -> AttributeState(value = result.data, readAt = System.currentTimeMillis())
                is AttributeRead.Failure -> it.copy(loading = false, progress = null, error = result.message)
            }
        }
        val data = (result as? AttributeRead.Value)?.data
        if (data != null && InterfaceClass.hasScaler(obj.classId)) {
            if (attribute == SCALER_UNIT) {
                ScalerUnit.parse(data)?.let { su -> _uiState.update { it.copy(scalers = it.scalers + (obj.obis to su)) } }
            } else if (withScaler && attribute == VALUE && obj.obis !in _uiState.value.scalers && obj.canRead(SCALER_UNIT)) {
                readNow(obj, SCALER_UNIT)
            }
        }
        return data
    }

    /**
     * Reads scaler_unit of each Register column of a profile that isn't known
     * yet, so the table can show units and scaled values.
     */
    private suspend fun readColumnScalers(obj: CosemObject, columns: List<CaptureObject>) {
        val missing = columns.filter {
            InterfaceClass.hasScaler(it.classId) && it.attribute == VALUE && it.obis !in _uiState.value.scalers &&
                find(it.obis.toString())?.canRead(SCALER_UNIT) != false
        }.distinctBy { it.obis }
        missing.forEachIndexed { i, column ->
            setBuffer(obj) { it.copy(progress = "Reading units ${i + 1}/${missing.size}...") }
            val target = CosemAttribute(column.classId, column.obis, SCALER_UNIT, "${column.label}.scaler_unit")
            val su = (dlmsRepository.read(target) as? AttributeRead.Value)?.data?.let(ScalerUnit::parse) ?: return@forEachIndexed
            _uiState.update { it.copy(scalers = it.scalers + (column.obis to su)) }
        }
    }

    /**
     * Reads a Profile Generic buffer the way Gurux offers it ([BufferSelection]).
     * capture_objects (the column headers and the clock column a date range
     * filters on) and entries_in_use are read first.
     */
    fun readBuffer(obj: CosemObject, selection: BufferSelection) {
        if (_uiState.value.buffers[obj.obis]?.loading == true) return
        viewModelScope.launch {
            setBuffer(obj) { it.copy(loading = true, error = null, progress = "Reading columns...") }
            val columns = _uiState.value.buffers[obj.obis]?.columns
                ?: readNow(obj, CAPTURE_OBJECTS)?.let { runCatching { CosemStructures.captureObjects(it) }.getOrNull() }
            val entries = (readNow(obj, ENTRIES_IN_USE) as? DlmsData.Integer)?.value
            columns?.let { readColumnScalers(obj, it) }

            val access = when (selection) {
                BufferSelection.All -> null
                is BufferSelection.Entries ->
                    SelectiveAccess.byEntry(selection.first, selection.first + selection.count - 1)
                is BufferSelection.Range, is BufferSelection.LastDays -> {
                    val (from, to) = selection.dateRange()
                    // Filter on the clock column; Gurux uses the first capture object, which is the clock here too.
                    val restricting = columns?.firstOrNull { it.classId == InterfaceClass.CLOCK } ?: columns?.firstOrNull()
                    if (restricting == null) {
                        setBuffer(obj) { it.copy(loading = false, progress = null, error = "No columns to filter by date") }
                        return@launch
                    }
                    SelectiveAccess.byRange(restricting, from, to)
                }
            }
            setBuffer(obj) { it.copy(progress = "Reading ${selection.describe()}...", entriesInUse = entries) }
            when (val result = dlmsRepository.read(obj.attribute(BUFFER), access) { p -> setBuffer(obj) { it.copy(progress = p) } }) {
                is AttributeRead.Value -> {
                    val rows = (result.data as? DlmsData.Array)?.items?.map { row ->
                        (row as? DlmsData.Structure)?.items ?: listOf(row)
                    }
                    setBuffer(obj) {
                        it.copy(
                            loading = false, progress = null, columns = columns, rows = rows,
                            error = if (rows == null) "Buffer is not an array: ${result.data.display().take(80)}" else null,
                        )
                    }
                }
                is AttributeRead.Failure -> setBuffer(obj) {
                    it.copy(loading = false, progress = null, columns = columns, error = result.message)
                }
            }
        }
    }

    /**
     * Switches the meter's relay with remote_disconnect / remote_reconnect,
     * then reads control state back, and output state once the relay has moved.
     */
    fun switchRelay(control: CosemObject, reconnect: Boolean) {
        viewModelScope.launch {
            val verb = if (reconnect) "Reconnect" else "Disconnect"
            _uiState.update { it.copy(relayMessage = "$verb...") }
            val error = dlmsRepository.invoke(DisconnectControl.method(control.obis, reconnect), DisconnectControl.PARAMETER)
            _uiState.update { it.copy(relayMessage = if (error == null) "$verb sent" else "$verb refused: $error") }
            if (error == null) {
                readNow(control, DisconnectControl.CONTROL_STATE)
                // The relay itself switches seconds after the command (seen on the FT5AB: an
                // immediate read still showed the old output_state).
                delay(RELAY_SETTLE_MS)
                readNow(control, DisconnectControl.OUTPUT_STATE)
            }
        }
    }

    /**
     * Sets [clock]'s time to the phone's local time. Keeps the deviation the
     * meter reported in its last time reading, if any.
     */
    /**
     * Writes [value] (the new value, of the type read from the meter) to one
     * attribute, then reads it back so the page shows what the meter now holds.
     */
    fun writeAttribute(obj: CosemObject, attribute: Int, value: DlmsData) {
        val key = ObjectBrowserUiState.key(obj, attribute)
        if (_uiState.value.write?.let { it.key == key && it.busy } == true) return
        viewModelScope.launch {
            _uiState.update { it.copy(write = AttributeWrite(key, busy = true)) }
            val error = dlmsRepository.write(obj.attribute(attribute), Axdr.encode(value))
            if (error == null) readNow(obj, attribute, withScaler = false)
            _uiState.update { it.copy(write = AttributeWrite(key, busy = false, error = error)) }
        }
    }

    fun clearWrite() = _uiState.update { it.copy(write = null) }

    fun setClockToPhoneTime(clock: CosemObject) {
        viewModelScope.launch {
            _uiState.update { it.copy(clockMessage = "Setting clock...") }
            val current = _uiState.value.attribute(clock, TIME).value as? DlmsData.OctetString
            val deviation = current?.let { ClockValue.deviationOf(it.bytes) } ?: ClockValue.DEVIATION_UNSPECIFIED
            val now = LocalDateTime.now().withNano(0)
            val error = dlmsRepository.write(clock.attribute(TIME), ClockValue.encode(now, deviation))
            _uiState.update {
                it.copy(clockMessage = if (error == null) "Clock set to $now" else "Clock not set: $error")
            }
            if (error == null) readNow(clock, TIME)
        }
    }

    fun dismissClockMessage() = _uiState.update { it.copy(clockMessage = null) }

    private fun setAttribute(key: String, change: (AttributeState) -> AttributeState) =
        _uiState.update { it.copy(attributes = it.attributes + (key to change(it.attributes[key] ?: AttributeState()))) }

    private fun setBuffer(obj: CosemObject, change: (ProfileBufferState) -> ProfileBufferState) =
        _uiState.update { it.copy(buffers = it.buffers + (obj.obis to change(it.buffers[obj.obis] ?: ProfileBufferState()))) }

    companion object {
        const val VALUE = 2

        /** Classes whose attribute 2 is the one value worth showing in a list. */
        val VALUE_CLASSES = setOf(
            InterfaceClass.DATA, InterfaceClass.REGISTER, InterfaceClass.EXTENDED_REGISTER,
            InterfaceClass.CLOCK, InterfaceClass.DISCONNECT_CONTROL,
        )

        /**
         * Attributes "Read all values" reads for [obj]: its value for [VALUE_CLASSES],
         * otherwise every attribute this client may read except logical_name and the
         * ones too big for a list-wide read (a Profile Generic buffer, Association LN's
         * object_list, about 18 s each); logical_name alone when nothing else is readable.
         */
        fun listAttributes(obj: CosemObject): List<Int> {
            if (obj.classId in VALUE_CLASSES) return if (obj.canRead(VALUE)) listOf(VALUE) else emptyList()
            val heavy = when (obj.classId) {
                InterfaceClass.PROFILE_GENERIC -> setOf(BUFFER)
                InterfaceClass.ASSOCIATION_LN -> setOf(2)
                else -> emptySet()
            }
            val readable = (2..InterfaceClass.attributeCount(obj.classId)).filter { it !in heavy && obj.canRead(it) }
            // Some objects allow nothing but logical_name (the FT5AB's Script Tables refuse
            // scripts to US and MR); reading that at least shows the object answers.
            return readable.ifEmpty { if (obj.canRead(1)) listOf(1) else emptyList() }
        }
        const val RELAY_SETTLE_MS = 3000L
        const val SCALER_UNIT = 3
        const val BUFFER = 2
        const val CAPTURE_OBJECTS = 3
        const val ENTRIES_IN_USE = 7
        const val TIME = 2
    }
}
