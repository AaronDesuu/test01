# FT5AB Migration Plan

Port MeterLinkIND from the **F5LWF** meter (HDLC) to the **FT5AB** meter (DLMS wrapper over the Dialog DSPS BLE module `BLE_FT5AB`).

Written 2026-10-31 from an analysis of the reference material in `app/sampledata/BLref/`.

## Before starting (repo housekeeping)

- **Sync the latest commits.** This repo stopped at `7c25d7f`. The commits `a04bd8e` (package rename, AGP 9.4.1) and `0334169` (AGP 9 built-in Kotlin/KSP) exist only in `..\MeterLink` and were never pushed. Bring them in with:
  ```powershell
  git fetch ..\MeterLink master
  git merge --ff-only FETCH_HEAD   # run `git restore .idea` first if it complains
  ```
- **Don't commit the BLE binaries.** `app/sampledata/BLref/BLE_FT5AB` is about 128 MB (.exe/.bin/.img). Unstage it and add `app/sampledata/BLref/BLE_FT5AB/` to `.gitignore`. The firmware `.c`/`.h` files are small and fine to keep.
- **Shift-JIS encoding.** The firmware sources are Shift-JIS (CP932). Read them with `iconv -f CP932 -t UTF-8 <file>`.

## Reference material

| Path | Contents |
|---|---|
| `app/sampledata/BLref/hrdtushin.c` | Transport layer: wrapper (`_wrapperR/_wrapperS`), HDLC, local `$` protocol, General Block Transfer streaming (`wrapCtrl`) |
| `app/sampledata/BLref/acse.c` | Association request/response handling: application context, mechanism selection, user information |
| `app/sampledata/BLref/aes_gcm.c` | Security Suite 0 (AES-GCM), associated data, key installation, HLS secret |
| `app/sampledata/BLref/dlms.c` | `USERS[]` client table, `Rank()`, unit/scaler table, firmware version string |
| `app/sampledata/BLref/xdlms.c` | `g_ist[]` object table (class + OBIS) at about line 503; GET/SET/ACTION handlers |
| `app/sampledata/BLref/include/xdlms.h` | `IST_*` object indices (1..457), security mechanism constants |
| `app/sampledata/BLref/BLE_FT5AB/projects/target_apps/dsps/` | DA14531 DSPS firmware (GATT table in `common/src/profiles/spss/user_sps_config.h`, config in `dsps_device/src/config/user_config.h`) |
| `C:\Users\fmc02\Desktop\FT5ABV1_TOOL(GMR)\` | Fuji PC factory tool (`Factory.exe`), its logs and screenshots |

## What changes from F5LWF to FT5AB

| Area | F5LWF (current app) | FT5AB |
|---|---|---|
| **BLE GATT** | Service `b973f2e0-b19e-11e2-9e96-0800200c9a66` | DSPS service `bdcede40-50d8-4eea-9afd-9218bf8cc760`. Notify (TX) `…c761`, write-without-response (RX) `…c762`, flow control `…c763` (meter sends `0x01` = resume, `0x02` = pause). It still **advertises** the old `b973f2e0` UUID. Device name is `FT5AB`. |
| **BLE throughput** | One notification = one frame | Characteristics hold up to 250 bytes and the meter UART runs at 19200 baud. Frames arrive split across notifications. |
| **Transport** | HDLC (SNRM/UA, `7E` flags, CRC) | Wrapper: `00 01 │ client port (2 bytes) │ server port 00 01 │ length (2 bytes) │ APDU`. No SNRM and no CRC. The second header byte must be 1..3 and is echoed back. Large responses may stream as several General Block Transfer (`E0`) frames with no acknowledgement between them. |
| **Clients** | Address `7f`, rank `03` | IS 15959 client table. Ports: Public 0x10, Meter Reader 0x20, Utility Settings/Admin 0x30, Push 0x40, Firmware 0x50, IHD 0x60, Super 0x3F (HDLC form `7f`). The meter computes `(wPort<<1)|1` and compares it with `USERS[].sys.maddr`. Super must send the calling AP title `"Super   "` and uses the master key. |
| **Security** | Non-standard: the challenge is used as the AES-GCM associated data | Standard Security Suite 0: the associated data is the security byte + authentication key (`aes_gcm.c:set_aad_key`). Encryption key, authentication key and LLS password are separate per client. |
| **Authentication** | Admin uses GMAC; Reader uses LLS with no encryption | Public: plain LN context, no security. Meter Reader / Push / IHD: **encrypted LN context** + LLS password + encrypted initiate, encryption only (SC `0x20`). Admin / FW / Super: encrypted context + HLS method 2 (AES-ECB of the meter's challenge with the HLS secret) or method 5 (GMAC), encrypted and authenticated (SC `0x30`). The meter accepts both global and dedicated keys. |
| **Objects** | 99 objects in `DLMS.java` | 457 objects in a different numbering (`xdlms.c g_ist[]`). Includes block/daily load profiles, billing, event logs 1–7, nameplate, instantaneous values with scaler profiles, disconnect control and load limit. |

**Decision:** replace the `dlms/DLMS.java` black box with a Kotlin DLMS stack we own and can unit-test. Its framing, security scheme, handshake and object table are all wrong for FT5AB. Also remove the unused `data/protocol/DlmsProtocol.kt` and `HdlcLayer.kt`.

## Phased plan

### Phase 1: BLE layer (`BleRepository`)
- Switch to the DSPS service and characteristic UUIDs.
- Subscribe to the flow-control characteristic and pause writes while the meter reports `0x02`.
- Request a larger MTU and split each write into MTU−3 byte pieces sent back to back. The wrapper has a timeout between characters, so pieces can't sit idle.
- Replace "one notification = one frame" with a reassembler that builds complete wrapper frames from the 8-byte header's length field.
- Raise timeouts for 19200 baud: about 0.5 ms per byte, so an 800-byte response takes about 0.4 s on the wire.
- Scanner: change the `"WF"` name filter to `"FT5AB"`, or filter by the advertised service UUID.

### Phase 2: New Kotlin DLMS stack (unit-tested)
1. `WrapperCodec`: encode and decode frames, including `E0` block-transfer reassembly.
2. `Axdr` and `Ber`: data encode/decode, plus the association request and response.
3. `SecuritySuite0`: AES-GCM via `javax.crypto`, a frame counter that only increases, HLS method 2, GMAC (method 5), global and dedicated keys.
4. `Association`: association request/response → HLS reply on the current-association object (`0.0.40.0.0.255`, method 1) when HLS is used → release request on disconnect.
5. `CosemClient`: GET (normal, with-block, with-list, selective access by range or entry), SET and ACTION. Encryption is applied automatically according to the session.
6. `ObjectRegistry`: generated by a script from `xdlms.c g_ist[]` (index, class, OBIS, name) so it can't drift from the firmware.

### Phase 3: Repository and ViewModels
- `DlmsRepository` keeps its `Flow<MeterDataResult>` interface and the operation mutex. `establishConnection()` loses the SNRM/UA step.
- Map each feature to its FT5AB object:
  - Serial number → 27 (`0.0.96.1.0.255`)
  - Clock → 410 (`0.0.1.0.0.255`, class 8, attribute 2)
  - Demand reset → 412 (MDI reset / end-of-billing script table `0.0.10.0.1.255`, ACTION method 1)
  - Load profile → 408 (block, `1.0.99.1.0.255`) and 409 (daily, `1.0.99.2.0.255`)
  - Billing → 407 (`1.0.98.1.0.255`)
  - Instantaneous values → 401 (`1.0.94.91.0.255`) plus scaler profiles 402–406
  - Events → 392–398 (`0.0.99.98.0..6.255`)
  - Nameplate → 391
  - New feature candidates: disconnect control (456) and load limit (457)
- **Factory screen:** meter type, missing neutral, power network and calibration have no direct equivalent. Likely candidates are manufacturer objects 60–76 (calibration 63–67, settings bit array 70, detect-event setting 76). Their data formats still need confirming.
- Values are now register + scaler/unit rather than pre-formatted strings, so `CsvExporter` and the display cards need updating.

### Phase 4: Settings and UI
- Replace the free-text address/rank with a **client profile** picker (Public / MR / US / FW / Super). Each profile stores its LLS password, encryption key, authentication key, HLS secret and system title in encrypted storage, not plain `SharedPreferences`.
- The current defaults (`7f` + rank `03`) would not authenticate on FT5AB. Address `7f` is Super, which needs HLS.
- Rename F5LWF → FT5AB in user-facing strings, and update `CLAUDE.md`.

### Phase 5: Testing
- Unit tests for wrapper framing, GCM against published DLMS test vectors, the association-request byte layout and GBT reassembly.
- Then on a real meter, one pass per client profile: connect, read serial, read a profile with block transfer, set the clock, demand reset.
- Finally, delete `DLMS.java`.

## Open questions (answers needed)
1. **F5LWF support:** drop it, or support both meters behind a meter-type switch?
2. **Credentials:** default keys, LLS passwords and HLS secrets per client. These aren't in the provided sources: `read_global_key`, `read_auth_key`, `read_auth_val` and `read_master_key` are defined elsewhere. Which client will field users connect as (MR or US/Admin)?
3. **Factory functions:** use DLMS (Super/Admin) or the factory tool's ASCII `$` local protocol (`Z2`, `Z3`, `S`, `F3`, `NC`, `K04`…, framed `$ $ len cmd data`, handled by `_localR/_localS`)? Either way, the Fuji object spec for objects 60–76 is needed.
4. **Test vectors:** a DLMS session log from the PC tool with `cs/ce/cd/cr` lines would give exact vectors for the security code. The existing logs only show local-protocol traffic.

Phase 1 and the wrapper/codec parts of Phase 2 don't depend on these answers.
