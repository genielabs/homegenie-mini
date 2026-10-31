# Changelog

## Unreleased

- Hide Smart Display Restore Config during temporary BOOT 1/BOOT 2 setup; retain it for persistent on-screen Reconfigure. Temporary setup exits through an ordinary reset or power cycle.

- Harden Smart Display Bluetooth setup completion: clear and verify the persistent Reconfigure flag and one-shot request before Bluetooth teardown/restart, and synchronize the in-memory system mode. Keep the provisioning channel available if the exit cannot be saved. WPS also checks the finalization result before restarting. Production-method host regressions pass for display first-time/Reconfigure, BOOT modes, BLE/classic variants and storage failures. Maintainer confirmed Smart Display S3 tests after this fix and the temporary-screen correction.

- Complete temporary WPS immediately on its success event: read the received credentials directly from the station SDK, verify storage, then restart normally after one second. Remove the intermediate association/IP wait; normal startup performs the Wi-Fi connection. ESP32/ESP8266 host tests cover success without IP/previous credentials, full-length SDK fields, storage/read failures and repeated retries. Maintainer confirmed the final WPS save/restart/reconnection flow on color-light C3.

- Retry temporary WPS automatically after startup, pairing or credential-saving errors/timeouts. Keep the selected configuration mode and previous credentials, clear per-attempt events/state, and restart WPS after a nonblocking two-second pause. Reboot automatically only after the received WPS credentials are saved. ESP32/ESP8266 host retry/recovery regressions pass; repeated router/device retries are pending.

- Fix color-light crashes when increasing LED count at runtime. Defer resizing to the render loop, retain existing effect colors, release truncated colors, and roll back on allocation failure. Reuse individual LED handlers and guard inactive indices. Clip script ranges safely after shrinking, retain compact bytecode and restart playback. Host regressions pass with/without automation, including repeated size changes and allocation failures. Maintainer confirmed live LED-count changes on color-light C3.


- Add an optional color-light auxiliary controller LED configured through `auld-pin`, `auld-typ`, `auld-spd` and `auld-mod`. Default to status/effect replication; expose C2 with an app-selectable, persistent independent mode for on/off, level, color and transitions. Allocate hardware/module resources only when configured; support aux-only boards and reject shared LED-output pins. Keep C1, matrix rendering and provisioning blink policy unchanged. Production-class host regressions passed with/without a built-in RGB LED, as did a complete C3 7.1.3 build and targeted compilation of the two affected S3 8.0.0 translation units. Maintainer confirmed auxiliary controller LED operation; remaining per-controller checks are tracked in the roadmap.

- Fix color-light script coordinate spans and rectangles on vertically wired matrices. Compile selections along the physical wiring axis, retaining compact bytecode, physical-index semantics and the existing frame renderer. Resend scripts after changing the matrix layout. Native regressions passed (304 cases across all 16 layouts, rectangular matrices and the Pac-Man Ghost), as did an isolated C3 Arduino 2.x firmware build; device validation is pending.

- Pin all eight ESP32-C3 environments to `espressif32@7.1.3` (Arduino 2.x) to preserve internal heap for JavaScript automation. Keep the default ESP32 platform at `espressif32@~8.0.0` and ESP8266 at `espressif8266@2.6.3`. Share C3 platform/compiler settings and report the correct platform in SystemInfo; retain current firmware features and Bluetooth provisioning. Keep the optional no-Bluetooth diagnostic profile explicitly on 8.0.0. See [build platform policy](docs/build-platforms.md).

- Always select BOOT 1 (WPS only, Bluetooth off) on the first physical or serial setup request, with or without saved Wi-Fi credentials. A separate second long press clears only Wi-Fi credentials and enters BOOT 2 with Bluetooth enabled. Use the same sequence from Smart Display's guided setup; keep its on-screen actions unchanged. Ordinary offline startup remains unchanged. Preserve smart-display first-time BLE setup and its non-destructive Reconfigure/Restore flow; show router-WPS instructions during BOOT 1 instead of the Bluetooth app QR code. Host regressions pass for the actual button dispatcher with saved/empty credentials and Smart Display modes, first-time WPS, Bluetooth startup, LED timing and rendering. Maintainer confirmed the final configuration flow on color-light C3 and Smart Display S3 on 2026-10-31; other target-specific checks remain pending.

- Start WPS automatically in temporary BOOT 1/BOOT 2 configuration. Preserve previous credentials on failed/interrupted BOOT 1 attempts, retain the BOOT 2 reset, and automatically restart one second after successful WPS pairing and verified credential storage, without waiting for association or DHCP. Keep SDK event callbacks free of blocking waits and storage writes for this workflow. WPS host regressions and complete isolated color-light C3/ESP8266 builds passed; final C3 and Smart Display S3 configuration tests were confirmed by the maintainer; remaining target checks are tracked separately.

- Maintainer confirmed both BOOT indication/reset stages, normal offline light operation, WPS save/restart/reconnection, interrupted-WPS credential preservation and Bluetooth reset/provisioning on color-light C3.

- Add explicit temporary configuration in the core: BOOT long press and release preserves Wi-Fi credentials; a separate long press in configuration resets credentials. One-shot boot requests are consumed before services start. Keep smart-display first-time provisioning and persistent Reconfigure/Restore behavior. Add serial `#CONFIG:wifi-setup` for the same temporary entry.
- Permit normal local operation without saved Wi-Fi credentials and without network warnings. For configured Wi-Fi, restore historical 100 ms disconnected flashes and 50 ms connected pulses approximately every two seconds. Color-light limits connected pulses to its initial 20-second startup window and keeps loss signalling active until reconnected. Explicit configuration keeps its distinct regular/two-pulse patterns and takes precedence. Reconnect waits remain unchanged.
- Put Waves first in the color-light effect list, before Solid, and select it at startup; preserve named effect values and local controls without Wi-Fi. Keep status-LED feedback for sparse effects and the existing matrix power limit and per-pixel overrides.
- Add host regressions for configuration requests, button handling, indicators and the actual 16x16 renderer. Full isolated C3/ESP8266 builds and targeted S3 display source compilation passed; the complete display link was not validated after local Windows object-directory errors. Final C3 and Smart Display S3 configuration acceptance was confirmed by the maintainer; remaining target checks are listed in [configuration mode](docs/configuration-mode.md).

## 1.2.68

Prepared on 2026-10-22 from the current working tree and maintainer-reported build and hardware tests. The firmware version is 1.2.68. On 2026-10-24 the maintainer confirmed release CI and artifacts, and publication of the updated firmware through the website.

### Changed

- Move ESP32 builds to `espressif32@~8.0.0` and update LovyanGFX, IRremoteESP8266 and ESP32Servo for the new core. Keep compatibility guards for older cores and ESP8266, including camera PSRAM detection, HTTP buffer constants and request-handler signatures.
- Add the narrowly guarded Arduino Nano ESP32 3.3.12 C-linkage pin-remapping shim, preserving the existing Arduino-pin mapping. It does not apply to ESP32-S3-Zero.
- Prefer PSRAM for the LVGL memory pool, Duktape heaps and digital-clock sprites, with internal-memory fallback. Boards without the guarded PSRAM support retain their existing allocation path.
- Format logger timestamps directly without `strftime`, reducing linked IRAM requirements and avoiding overlapping source/destination formatting.

### Fixed

- Restore independent smart-motor S3 outputs with a repository-local ESP32Servo 3.2.1: correct MCPWM timer/output routing, LEDC bookkeeping, shared-timer initialization and vacant-operator reuse, and use a 14-bit S3 servo default. Maintainer verified the combined four-wheel/four-servo robot-car and arm. The remaining wheel-neutral issue was traced to a stored angle maximum of 80 instead of 180 and resolved by correcting the configuration. Application directions, angle-to-level mapping and initial level 50 remain unchanged; angular limits are not overwritten at every boot.

- Keep the JavaScript HTTP transport alive until `HTTPClient` destruction for GET and POST. Earlier event-lifetime/locking experiments were reverted; they are not the final motion-triggered crash fix.
- Read the factory MAC for SSDP identity before Wi-Fi initialization, and support the new Arduino HTTP request-handler overloads.
- Seed the API/event clock from a valid retained RTC and synchronize it after explicit time-setting commands.
- Protect camera capture, settings, shutdown and SD access. Display and HTTP tasks share a compatible JPEG with one lease per task; the last reader returns it to the driver. The mutex is not held during HTTP transmission. Incompatible resolution/quality requests wait for existing readers, and invalid frames are checked before use. The intermediate fully serialized implementation was superseded after it reduced simultaneous-preview FPS.
- Guard RF interrupt detachment when no valid receiver interrupt has been assigned.

### Confirmed tests and limits

- All 45 PlatformIO environments compiled at the initial compatibility checkpoint; the maintainer subsequently confirmed successful complete builds after the final firmware changes. The maintainer subsequently confirmed release CI and ZIP artifacts on 2026-10-24.
- Maintainer tested IR, RF, matrix LEDs on color-light C3, JavaScript scheduling, SSDP discovery, HTTP access and device time synchronization. The IR/RF device is an ESP32-S3-Zero; it does not drive the 256-LED matrix.
- After the HTTP transport-lifetime fix, the S3 motion-automation device reached 29,435.480 seconds of uptime (about 8 h 10 min), with 188,948 bytes free DRAM at that sample. This is a successful observation interval, not proof that every intermittent crash or Wi-Fi failure is resolved.
- Display PSRAM changes recovered at least 30 KB of DRAM in maintainer testing. With Duktape also using PSRAM, the tested light-control activity had about 117 KB free DRAM at rest and 113 KB during script execution. The earlier tested PHY timer allocation crash did not recur in that test.
- Maintainer confirmed local and remote camera operation, then reproduced simultaneous-access crashes. The first protection fix stopped those crashes but reduced FPS; after restoring safe frame sharing, the maintainer reported that simultaneous viewing now works well.
- Native tests against extracted production methods passed shared-frame ownership, final-reader release, invalid captures, reconfiguration, SD save and 500 concurrent acquisition attempts. Allocator tests passed PSRAM/fallback and allocation-failure cases. These are host tests, not firmware or long-running device acceptance.
- Maintainer confirmed successful firmware upload on ESP32-S3-CAM without IO0, including the first attempt after USB disconnect/reconnect, and on ESP32-S3-Zero and ESP32-C3-Zero/Mini. The tested classic ESP32-CAM requires manual IO0 with both the website and PlatformIO; upload completes when IO0 is used.
- Full builds, camera acceptance and motor hardware checks are completed according to maintainer confirmation and the release checklist. Final CI/artifact packaging was also confirmed by the maintainer on 2026-10-24. See [ROADMAP.md](ROADMAP.md).

### Companion configurator

- The website now applies robot-car/arm angle and speed presets during explicit device initialization, including the tested gripper settings; this is separate from firmware boot behavior. The maintainer confirmed that reinitialization restores a deliberately incorrect wheel setting.
- Website upload fixes include the classic ESP32-CAM bootloader offset and the esptool-js 0.7.0 integration. These are website changes, not dependencies embedded in the firmware. The tested classic CAM's manual IO0 requirement remains documented above.
