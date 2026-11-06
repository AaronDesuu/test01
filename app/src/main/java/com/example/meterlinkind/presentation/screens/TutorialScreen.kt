package com.example.meterlinkind.presentation.screens

import androidx.compose.ui.draw.clip
import androidx.compose.runtime.setValue
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.foundation.clickable
import androidx.activity.compose.BackHandler
import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.animateDpAsState
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material.icons.filled.Bluetooth
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.School
import androidx.compose.material.icons.filled.SignalCellularAlt
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.meterlinkind.R
import com.example.meterlinkind.data.protocol.ClientProfile
import com.example.meterlinkind.presentation.components.OnlineColor
import com.example.meterlinkind.presentation.components.RoleBadge
import kotlinx.coroutines.launch

/** The languages the tutorial is written in; [code] is what Settings stores. */
private enum class Language(val code: String, val label: String) {
    EN("en", "EN"),
    JA("ja", "日本語"),
}

/** One tutorial page: its title and text in each language, and its picture. */
private class TutorialPage(
    val title: Map<Language, String>,
    val body: Map<Language, String>,
    val picture: @Composable () -> Unit,
)

private fun page(enTitle: String, enBody: String, jaTitle: String, jaBody: String, picture: @Composable () -> Unit) =
    TutorialPage(
        title = mapOf(Language.EN to enTitle, Language.JA to jaTitle),
        body = mapOf(Language.EN to enBody, Language.JA to jaBody),
        picture = picture,
    )

// The pictures keep the app's own English labels, so the Japanese text names
// buttons and pages as they appear on screen, in 「」.
private val PAGES = listOf(
    page(
        "Welcome to Meter Link",
        "Read and set up FT5AB meters from your phone, over Bluetooth. This takes about a minute.",
        "Meter Linkへようこそ",
        "Bluetoothでつないで、スマホからFT5ABメーターの読み取りや設定ができます。使い方の説明は1分ほどです。",
    ) { WelcomePicture() },
    page(
        "Log in",
        "What you can do depends on your login. Utility Settings (US) reads the meter and changes its settings. " +
            "Meter Reader (MR) only reads. Leave the app unused for 3 days and you'll need to log in again.",
        "ログインについて",
        "ログインの種類で、使える機能が変わります。「Utility Settings（US）」は読み取りと設定変更、" +
            "「Meter Reader（MR）」は読み取りのみです。3日間アプリを使わないと、ログインし直しになります。",
    ) { LoginPicture() },
    page(
        "Check the security keys",
        "Next you'll see the password and keys for your login. They have to match the meter's, or it won't let you in. " +
            "Type them as 16 characters or 32 hex digits. You can change them any time in Settings › Security keys.",
        "セキュリティキーを確認",
        "ログインすると、パスワードと各キーの画面が出ます。メーターと同じ値でないとつながらないので、ここで確認してください。" +
            "入力は16文字（ASCII）か、16進数32桁です。あとから「Settings › Security keys」でも変えられます。",
    ) { KeysPicture() },
    page(
        "Connect to a meter",
        "On the Meter page, tap Scan for Meters and pick your FT5AB. Weak signal? Move closer. " +
            "Once you're connected, the meter's name stays at the top right of every page.",
        "メーターにつなぐ",
        "「Meter」画面で「Scan for Meters」をタップして、FT5ABを選びます。電波が弱いときは、メーターに近づいてください。" +
            "つながっている間は、どの画面でも右上にメーター名が出ます。",
    ) { ConnectPicture() },
    page(
        "Get the object list",
        "The first time you open a meter's objects, the app reads its object list from the meter. " +
            "That takes about 20 seconds, once. For another FT5AB, you can reuse a list you already have.",
        "オブジェクトリストを読み込む",
        "メーターのオブジェクトを初めて開くと、メーターからオブジェクトリストを読み込みます。" +
            "20秒ほどかかりますが、最初の1回だけです。2台目からのFT5ABでは、読み込み済みのリストを使い回せます。",
    ) { ObjectListPicture() },
    page(
        "Read values",
        "Pick a type of object from the menu, like Register. Read all values reads the whole list. " +
            "Read on a row reads just that one. Tap an object to see all its attributes.",
        "値を読む",
        "メニューから「Register」などの種類を選びます。「Read all values」で一覧の値をまとめて、" +
            "各行の「Read」でその1件だけを読みます。オブジェクトをタップすると、属性を全部見られます。",
    ) { ReadPicture() },
    page(
        "Change a value",
        "A pencil means your login can change that attribute. Tap it and type the new value. " +
            "Next shows what will be written, and Write sends it. The app then reads it back, so you see what the meter actually holds.",
        "値を書き換える",
        "鉛筆マークの付いた属性は、今のログインで書き換えられます。タップして新しい値を入れ、" +
            "「Next」で書き込む内容を確かめて、「Write」で送信します。送信後に読み直すので、メーターに実際に入った値がその場で分かります。",
    ) { EditPicture() },
    page(
        "You're ready",
        "That's it. To see this again, go to Settings › About › Tutorial.",
        "これで準備OK",
        "説明は以上です。もう一度見たいときは、「Settings › About › Tutorial」から開けます。",
    ) { ReadyPicture() },
)

/** The tutorial's own buttons and page count. */
private class Labels(
    val pageOf: (Int, Int) -> String,
    val skip: String,
    val back: String,
    val next: String,
    val getStarted: String,
    val done: String,
)

private val LABELS = mapOf(
    Language.EN to Labels({ n, total -> "$n of $total" }, "Skip", "Back", "Next", "Get started", "Done"),
    Language.JA to Labels({ n, total -> "$n / $total" }, "スキップ", "戻る", "次へ", "はじめる", "完了"),
)

/**
 * How to use the app, in eight swipeable pages, in English or Japanese. Shown
 * before login on the first launch ([firstLaunch]: Skip and Get started) and
 * from Settings (Done). [initialLanguage] is a [Language.code]; a switch at the
 * top changes it and reports the new code to [onLanguageChange].
 * [onFinish] runs when the user finishes or skips.
 */
@Composable
fun TutorialScreen(
    firstLaunch: Boolean,
    initialLanguage: String,
    onLanguageChange: (String) -> Unit,
    onFinish: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val pager = rememberPagerState { PAGES.size }
    val scope = rememberCoroutineScope()
    var language by rememberSaveable {
        mutableStateOf(Language.entries.firstOrNull { it.code == initialLanguage } ?: Language.EN)
    }
    val labels = LABELS.getValue(language)
    val last = pager.currentPage == PAGES.lastIndex
    // Back goes to the previous page; on the first page it leaves as usual.
    BackHandler(enabled = pager.currentPage > 0) { scope.launch { pager.animateScrollToPage(pager.currentPage - 1) } }

    Column(modifier = modifier.fillMaxSize()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(start = 24.dp, end = 12.dp, top = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                labels.pageOf(pager.currentPage + 1, PAGES.size),
                style = MaterialTheme.typography.labelLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.weight(1f)
            )
            LanguageSwitch(language) {
                language = it
                onLanguageChange(it.code)
            }
            if (firstLaunch) TextButton(onClick = onFinish) { Text(labels.skip) }
        }

        HorizontalPager(state = pager, modifier = Modifier.weight(1f)) { index ->
            val page = PAGES[index]
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .verticalScroll(rememberScrollState())
                    .padding(horizontal = 24.dp, vertical = 8.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(260.dp)
                        .background(MaterialTheme.colorScheme.surfaceContainerLow, RoundedCornerShape(24.dp)),
                    contentAlignment = Alignment.Center
                ) { page.picture() }
                Text(page.title.getValue(language), style = MaterialTheme.typography.headlineSmall)
                Text(page.body.getValue(language), style = MaterialTheme.typography.bodyLarge, color = MaterialTheme.colorScheme.onSurfaceVariant)
            }
        }

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(start = 24.dp, end = 16.dp, top = 8.dp, bottom = 16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            PageDots(count = PAGES.size, current = pager.currentPage, modifier = Modifier.weight(1f))
            if (pager.currentPage > 0) {
                TextButton(onClick = { scope.launch { pager.animateScrollToPage(pager.currentPage - 1) } }) { Text(labels.back) }
            }
            Button(onClick = { if (last) onFinish() else scope.launch { pager.animateScrollToPage(pager.currentPage + 1) } }) {
                Text(if (!last) labels.next else if (firstLaunch) labels.getStarted else labels.done)
            }
        }
    }
}

/** EN | 日本語, the selected one filled. */
@Composable
private fun LanguageSwitch(selected: Language, onSelect: (Language) -> Unit) {
    Row(
        Modifier
            .border(1.dp, MaterialTheme.colorScheme.outlineVariant, CircleShape)
            .clip(CircleShape)
    ) {
        // Same size for both halves: the Japanese font is taller and wider than "EN".
        Language.entries.forEach { language ->
            val on = language == selected
            Box(
                modifier = Modifier
                    .size(width = 64.dp, height = 32.dp)
                    .background(if (on) MaterialTheme.colorScheme.primaryContainer else Color.Transparent)
                    .clickable(enabled = !on) { onSelect(language) },
                contentAlignment = Alignment.Center
            ) {
                Text(
                    language.label,
                    style = MaterialTheme.typography.labelMedium,
                    color = if (on) MaterialTheme.colorScheme.onPrimaryContainer else MaterialTheme.colorScheme.onSurfaceVariant,
                    maxLines = 1
                )
            }
        }
    }
}

/** A dot per page; the current one stretches into a bar. */
@Composable
private fun PageDots(count: Int, current: Int, modifier: Modifier = Modifier) {
    Row(modifier = modifier, horizontalArrangement = Arrangement.spacedBy(6.dp)) {
        repeat(count) { i ->
            val width by animateDpAsState(if (i == current) 20.dp else 7.dp, label = "dot width")
            val color by animateColorAsState(
                if (i == current) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.outlineVariant,
                label = "dot color"
            )
            Box(Modifier.size(width = width, height = 7.dp).background(color, CircleShape))
        }
    }
}

// --- Pictures: small, non-interactive versions of the app's own parts ------------------

@Composable
private fun Picture(content: @Composable ColumnScope.() -> Unit) {
    Column(
        modifier = Modifier.widthIn(max = 260.dp).padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp),
        content = content
    )
}

@Composable
private fun MiniCard(content: @Composable () -> Unit) {
    Surface(shape = RoundedCornerShape(14.dp), color = MaterialTheme.colorScheme.surface, shadowElevation = 1.dp, modifier = Modifier.fillMaxWidth()) {
        Row(modifier = Modifier.padding(10.dp), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            content()
        }
    }
}

@Composable
private fun Dot(icon: ImageVector, background: Color, tint: Color) {
    Box(Modifier.size(32.dp).background(background, CircleShape), contentAlignment = Alignment.Center) {
        Icon(icon, contentDescription = null, tint = tint, modifier = Modifier.size(18.dp))
    }
}

@Composable
private fun Lines(title: String, subtitle: String, modifier: Modifier = Modifier, mono: Boolean = false) {
    Column(modifier) {
        Text(title, style = MaterialTheme.typography.titleSmall)
        Text(
            subtitle,
            style = MaterialTheme.typography.bodySmall,
            fontFamily = if (mono) FontFamily.Monospace else null,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
    }
}

@Composable
private fun MiniField(label: String, value: String) {
    Column(
        Modifier
            .fillMaxWidth()
            .border(1.dp, MaterialTheme.colorScheme.outline, RoundedCornerShape(8.dp))
            .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(8.dp))
            .padding(horizontal = 10.dp, vertical = 6.dp)
    ) {
        Text(label, style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        Text(value, style = MaterialTheme.typography.bodyMedium, fontFamily = FontFamily.Monospace)
    }
}

@Composable
private fun MiniButton(text: String, filled: Boolean = true) {
    Box(
        Modifier
            .fillMaxWidth()
            .then(
                if (filled) Modifier.background(MaterialTheme.colorScheme.primary, CircleShape)
                else Modifier.border(1.dp, MaterialTheme.colorScheme.outline, CircleShape)
            )
            .padding(vertical = 8.dp),
        contentAlignment = Alignment.Center
    ) {
        Text(
            text,
            style = MaterialTheme.typography.labelLarge,
            color = if (filled) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.primary
        )
    }
}

@Composable
private fun WelcomePicture() {
    Box(
        Modifier.size(128.dp).background(MaterialTheme.colorScheme.primary, RoundedCornerShape(36.dp)),
        contentAlignment = Alignment.Center
    ) {
        Image(painterResource(R.drawable.ic_meterlinkind_logo), contentDescription = null, modifier = Modifier.size(108.dp))
    }
}

@Composable
private fun LoginPicture() = Picture {
    MiniCard {
        Dot(Icons.Default.Check, MaterialTheme.colorScheme.primary, MaterialTheme.colorScheme.onPrimary)
        Lines("Utility Settings", "Read and change", Modifier.weight(1f))
        RoleBadge(ClientProfile.UTILITY_SETTINGS)
    }
    MiniCard {
        Dot(Icons.Default.Check, MaterialTheme.colorScheme.surfaceVariant, MaterialTheme.colorScheme.onSurfaceVariant)
        Lines("Meter Reader", "Read only", Modifier.weight(1f))
        RoleBadge(ClientProfile.METER_READER)
    }
}

@Composable
private fun KeysPicture() = Picture {
    Row(
        Modifier
            .fillMaxWidth()
            .border(1.dp, MaterialTheme.colorScheme.outline, CircleShape)
    ) {
        Text(
            "ASCII (16)",
            style = MaterialTheme.typography.labelMedium,
            textAlign = TextAlign.Center,
            modifier = Modifier
                .weight(1f)
                .background(MaterialTheme.colorScheme.secondaryContainer, RoundedCornerShape(topStartPercent = 50, bottomStartPercent = 50))
                .padding(vertical = 5.dp)
        )
        Text("Hex (32)", style = MaterialTheme.typography.labelMedium, textAlign = TextAlign.Center, modifier = Modifier.weight(1f).padding(vertical = 5.dp))
    }
    MiniField("Authentication Password", "0000000000000000")
    MiniField("Block Cipher Key", "0000000000000000")
    MiniButton("Save & Continue")
}

@Composable
private fun ConnectPicture() = Picture {
    Row(
        Modifier
            .align(Alignment.End)
            .background(Color(0xFFD5F3E3), CircleShape)
            .padding(horizontal = 10.dp, vertical = 4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Box(Modifier.size(7.dp).background(OnlineColor, CircleShape))
        Spacer(Modifier.width(5.dp))
        Text("FT5AB", style = MaterialTheme.typography.labelMedium, color = Color(0xFF151928))
    }
    MiniCard {
        Lines("FT5AB", "48:23:35:0F:E2:CC", Modifier.weight(1f), mono = true)
        Column(horizontalAlignment = Alignment.End) {
            Icon(Icons.Default.SignalCellularAlt, contentDescription = null, tint = MaterialTheme.colorScheme.primary, modifier = Modifier.size(18.dp))
            Text("−42 dBm", style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }
    }
    MiniCard {
        Dot(Icons.Default.Check, MaterialTheme.colorScheme.primary, MaterialTheme.colorScheme.onPrimary)
        Column {
            Text("CONNECTED", style = MaterialTheme.typography.labelSmall, fontWeight = FontWeight.Bold, color = OnlineColor)
            Text("FT5AB", style = MaterialTheme.typography.titleSmall)
            Text("S/N: E00000003FT", style = MaterialTheme.typography.bodySmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
        }
    }
}

@Composable
private fun ObjectListPicture() = Picture {
    Surface(shape = RoundedCornerShape(20.dp), color = MaterialTheme.colorScheme.surface, shadowElevation = 2.dp) {
        Column(Modifier.padding(14.dp), verticalArrangement = Arrangement.spacedBy(8.dp), horizontalAlignment = Alignment.CenterHorizontally) {
            Text("New meter", style = MaterialTheme.typography.titleMedium)
            Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                Text("E00000099…", style = MaterialTheme.typography.labelSmall, fontFamily = FontFamily.Monospace)
                Icon(Icons.AutoMirrored.Filled.ArrowForward, contentDescription = null, modifier = Modifier.size(14.dp))
                Text(
                    "E00000003FT",
                    style = MaterialTheme.typography.labelSmall,
                    fontFamily = FontFamily.Monospace,
                    modifier = Modifier
                        .background(MaterialTheme.colorScheme.primaryContainer, RoundedCornerShape(5.dp))
                        .padding(horizontal = 4.dp, vertical = 1.dp)
                )
            }
            MiniButton("Use saved list")
            MiniButton("Read from meter", filled = false)
        }
    }
}

@Composable
private fun ReadPicture() = Picture {
    listOf(
        Triple("Voltage L1", "1.0.32.7.0.255", "239.12 V"),
        Triple("Current L1", "1.0.31.7.0.255", "4.82 A"),
        Triple("Frequency", "1.0.14.7.0.255", null),
    ).forEach { (name, obis, value) ->
        MiniCard {
            Lines(name, obis, Modifier.weight(1f), mono = true)
            if (value != null) {
                Text(value, style = MaterialTheme.typography.titleSmall)
            } else {
                Text("Read", style = MaterialTheme.typography.labelLarge, color = MaterialTheme.colorScheme.primary)
            }
        }
    }
    Row(
        Modifier
            .fillMaxWidth()
            .background(MaterialTheme.colorScheme.primary, RoundedCornerShape(12.dp))
            .padding(horizontal = 12.dp, vertical = 8.dp)
    ) {
        Text("Read all values", style = MaterialTheme.typography.labelLarge, color = MaterialTheme.colorScheme.onPrimary, modifier = Modifier.weight(1f))
        Text("202 objects", style = MaterialTheme.typography.labelMedium, color = MaterialTheme.colorScheme.onPrimary)
    }
}

@Composable
private fun EditPicture() = Picture {
    MiniCard {
        Column(Modifier.weight(1f)) {
            Text("8 · inactivity_time_out", style = MaterialTheme.typography.labelMedium, color = MaterialTheme.colorScheme.onSurfaceVariant)
            Text("10", style = MaterialTheme.typography.bodyMedium, fontFamily = FontFamily.Monospace)
        }
        Icon(Icons.Default.Edit, contentDescription = null, tint = MaterialTheme.colorScheme.primary)
    }
    Surface(shape = RoundedCornerShape(20.dp), color = MaterialTheme.colorScheme.surface, shadowElevation = 2.dp) {
        Column(Modifier.padding(14.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Text("Change inactivity_time_out", style = MaterialTheme.typography.titleSmall)
            MiniField("Number (long-unsigned)", "11")
            Row(Modifier.align(Alignment.End), horizontalArrangement = Arrangement.spacedBy(16.dp)) {
                Text("Cancel", style = MaterialTheme.typography.labelLarge, color = MaterialTheme.colorScheme.primary)
                Text("Next", style = MaterialTheme.typography.labelLarge, color = MaterialTheme.colorScheme.primary)
            }
        }
    }
}

@Composable
private fun ReadyPicture() = Picture {
    MiniCard {
        Dot(Icons.Default.School, MaterialTheme.colorScheme.primaryContainer, MaterialTheme.colorScheme.primary)
        Lines("Tutorial", "How to use Meter Link", Modifier.weight(1f))
    }
    MiniCard {
        Dot(Icons.Default.Info, MaterialTheme.colorScheme.primaryContainer, MaterialTheme.colorScheme.primary)
        Lines("Version", "0.10", Modifier.weight(1f))
    }
    MiniCard {
        Dot(Icons.Default.Bluetooth, MaterialTheme.colorScheme.primaryContainer, MaterialTheme.colorScheme.primary)
        Lines("Meter", "Scan for Meters", Modifier.weight(1f))
    }
}
