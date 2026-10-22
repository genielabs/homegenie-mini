# Changelog

## 1.2.68 (unreleased)

Prepared on 2026-10-22 from the current working tree and maintainer-reported build and hardware tests. The firmware version is set to 1.2.68; publication and final CI artifact verification are still pending.

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

- All 45 PlatformIO environments compiled at the initial compatibility checkpoint; the maintainer subsequently confirmed successful complete builds after the final firmware changes. Final release CI and ZIP artifacts still need verification.
- Maintainer tested IR, RF, matrix LEDs on color-light C3, JavaScript scheduling, SSDP discovery, HTTP access and device time synchronization. The IR/RF device is an ESP32-S3-Zero; it does not drive the 256-LED matrix.
- After the HTTP transport-lifetime fix, the S3 motion-automation device reached 29,435.480 seconds of uptime (about 8 h 10 min), with 188,948 bytes free DRAM at that sample. This is a successful observation interval, not proof that every intermittent crash or Wi-Fi failure is resolved.
- Display PSRAM changes recovered at least 30 KB of DRAM in maintainer testing. With Duktape also using PSRAM, the tested light-control activity had about 117 KB free DRAM at rest and 113 KB during script execution. The earlier tested PHY timer allocation crash did not recur in that test.
- Maintainer confirmed local and remote camera operation, then reproduced simultaneous-access crashes. The first protection fix stopped those crashes but reduced FPS; after restoring safe frame sharing, the maintainer reported that simultaneous viewing now works well.
- Native tests against extracted production methods passed shared-frame ownership, final-reader release, invalid captures, reconfiguration, SD save and 500 concurrent acquisition attempts. Allocator tests passed PSRAM/fallback and allocation-failure cases. These are host tests, not firmware or long-running device acceptance.
- Maintainer confirmed successful firmware upload on ESP32-S3-CAM without IO0, including the first attempt after USB disconnect/reconnect, and on ESP32-S3-Zero and ESP32-C3-Zero/Mini. The tested classic ESP32-CAM requires manual IO0 with both the website and PlatformIO; upload completes when IO0 is used.
- Full builds, camera acceptance and motor hardware checks are completed according to maintainer confirmation and the release checklist. The remaining release check is final CI/artifact packaging. See [ROADMAP.md](ROADMAP.md).

### Companion configurator

- The website now applies robot-car/arm angle and speed presets during explicit device initialization, including the tested gripper settings; this is separate from firmware boot behavior. The maintainer confirmed that reinitialization restores a deliberately incorrect wheel setting.
- Website upload fixes include the classic ESP32-CAM bootloader offset and the esptool-js 0.7.0 integration. These are website changes, not dependencies embedded in the firmware. The tested classic CAM's manual IO0 requirement remains documented above.
