# Meter Link release notes / リリースノート

## 0.10 (development build) — 2026-11-06

### English

The first build for testing. Meter Link reads and sets up FT5AB meters over Bluetooth. It's still in development, so expect rough edges. Version 1.00 will be the full release.

**What you can do**

- **Log in** as Utility Settings (US), who can read and change settings, or as Meter Reader (MR), who can only read. The login lasts until the app has gone unused for 3 days.
- **Connect** by scanning for nearby FT5AB meters. Signal strength shows next to each meter. Once you're connected, the meter's name stays at the top right of every page.
- **Browse the meter's objects** the way Gurux shows them: 17 object types, including Data, Register, Profile Generic, Clock and Disconnect Control. The object list comes from the meter itself. Reading it takes about 20 seconds the first time; after that the app keeps it, and another FT5AB can reuse it.
- **Read values** one at a time, or all of them with Read all values. Profiles (load survey, billing and so on) can be read by entry number, by the last few days, between two dates, or in full.
- **Change values.** Any attribute your login may write has a pencil. You see what will be written before it's sent, and the app reads it back afterwards. The clock can be set from the phone's time.
- **Keys stay on the phone.** Security keys are saved encrypted and left out of Google backups, so on a new phone you enter them again.
- **Tutorial** in English and Japanese. It opens on first launch and stays in Settings › About.

**Not there yet**

- Image Transfer (firmware update) is in the menu but turned off.
- Activity Calendar isn't supported.
- A few values show an odd scale, such as a 1-minute voltage average of 24000 V. We're checking these against the meter.

### 日本語

テスト用の最初のビルドです。Meter Linkは、BluetoothでFT5ABメーターの読み取りや設定ができるアプリです。まだ開発中なので、粗いところもあります。正式リリースはバージョン1.00の予定です。

**できること**

- **ログイン**：「Utility Settings（US）」は読み取りと設定変更、「Meter Reader（MR）」は読み取りのみ。3日間アプリを使わなければ、ログインし直しになります。
- **メーターにつなぐ**：近くのFT5ABを探して選ぶだけです。各メーターの横に電波の強さが出ます。つながっている間は、どの画面でも右上にメーター名が出ます。
- **オブジェクトを見る**：Guruxと同じ並びで、Data、Register、Profile Generic、Clock、Disconnect Controlなど17種類に対応しています。オブジェクトリストはメーターから直接読み込みます。初回は20秒ほどかかりますが、2回目からは保存したリストを使います。別のFT5ABでも使い回せます。
- **値を読む**：1件ずつでも、「Read all values」でまとめてでも読めます。Profile（ロードサーベイや請求データなど）は、件数指定・直近の日数・日時の範囲・全件から読み方を選べます。
- **値を書き換える**：書き込める属性には鉛筆マークが付きます。送信前に書き込む内容を確認でき、送信後はアプリが読み直すので、実際に入った値が分かります。時計はスマホの時刻に合わせられます。
- **キーはスマホの中だけに**：セキュリティキーは暗号化して保存し、Googleのバックアップには含めません。機種変更したときは、キーを入力し直してください。
- **チュートリアル**：英語と日本語に対応。初回起動時に開き、あとから「Settings › About」でも見られます。

**まだできないこと**

- Image Transfer（ファームウェア更新）はメニューにありますが、使えません。
- Activity Calendarには未対応です。
- 一部の値の桁がおかしく表示されます（1分平均電圧が24000 Vになるなど）。メーター側と照らし合わせて確認中です。
