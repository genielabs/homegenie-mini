# Roadmap

Status: 2026-10-31. This list separates implemented changes and observed results from pending release checks. See [CHANGELOG.md](CHANGELOG.md).

## Auxiliary controller LED

- [x] Add optional `auld-*` settings and C2 to color-light. Default to status/effect replication; provide a persistent `LED.mode` option for independent color control. Keep C1/matrix rendering and core blink rules, support boards without a built-in RGB indicator, and allocate the optional driver/module only when a separate GPIO is configured.
- Host regressions passed with/without a built-in RGB indicator; the C3 7.1.3 firmware build and affected S3 8.0.0 translation units compiled successfully. The complete S3 firmware link was not run to completion in this Windows environment.
- [x] Maintainer confirmed auxiliary controller LED operation after implementation. C3 GPIO 8 and S3 GPIO 9, type 82, are the selected controller wiring; defaults remain disabled on other installations.
- [ ] Complete any remaining per-controller checks of independent C2 transitions, mode persistence and C1 isolation before release.
- [ ] Consider a second strip/matrix output controlled by C2 in a future change; this first version drives only the auxiliary indicator.

## Current consolidation

- [x] Implement ESP32 Arduino 3.x compatibility while preserving guarded older-core and ESP8266 paths. Initial validation covered all 45 environments; the maintainer subsequently confirmed complete successful builds after the final firmware changes.
- [x] Correct HTTP transport lifetime in JavaScript helpers. Maintainer observed about 8 h 10 min of motion-triggered operation after the fix; retain intermittent-crash investigation if new evidence appears.
- [x] Restore SSDP discovery, HTTP access and consistent device/API time; maintainer confirmed these on the test device.
- [x] Move eligible LVGL, sprite and Duktape allocations to PSRAM with fallback; maintainer confirmed improved DRAM availability and the tested display/script/camera workflow.
- [x] Restore concurrent display/HTTP frame sharing with last-reader release. Maintainer confirmed the final simultaneous-preview behavior after the serialized version's FPS regression.
- [x] Complete the final environment build check after the PSRAM/shared-camera and compatibility changes. The maintainer confirmed all builds succeed; release CI and artifacts were subsequently confirmed by the maintainer on 2026-10-24.
- [x] Complete maintainer camera acceptance. Local/remote viewing and the final shared-frame implementation were confirmed working, and the maintainer marked the camera check complete. Preserve frame sharing and valid buffer lifetime; continue investigating if new failures appear.
- [x] Resolve and verify the smart-motor S3 eight-output regression. The local ESP32Servo fixes passed native tests and maintainer hardware checks on the combined four-wheel/four-servo robot-car and arm. A remaining wheel issue was resolved by correcting its stored 0-80 angle range to 0-180. Website initialization presets were updated and tested; firmware boot does not indiscriminately overwrite user limits.
- [x] Observe motion-triggered scripts, Wi-Fi availability, uptime and DRAM under normal use; the maintainer marked this acceptance check complete. Keep future investigation driven by new evidence, without reviving the reverted event/scheduler locking experiments.
- [x] Verify website uploads on S3-CAM after cold USB reconnect, S3-Zero and C3-Zero/Mini. Classic ESP32-CAM upload works with manual IO0, as with PlatformIO; automatic entry on that tested board is not confirmed.
- [x] Set firmware version to 1.2.68 and align documentation with confirmed results.
- [x] Commit the complete candidate, including vendored ESP32Servo and compatibility/memory helpers, and verify release CI builds and ZIP artifacts for 1.2.68. Commit `bbfa6a8` contains the consolidation; the maintainer confirmed CI and artifacts on 2026-10-24.

## Smart light: offline operation and configuration

Scope agreed with the maintainer on 2026-10-30: explicit configuration mode and connection-status signalling belong in the core and apply across firmware profiles. Smart-display must also retain its existing Reconfigure/Restore workflow and persistent configuration mode. The initial effect and effect-progress LED work are specific to examples/color-light.

- [x] Initialize local light controls, pixels and effects independently of saved Wi-Fi credentials. A device without configured Wi-Fi must be usable immediately, without automatically entering provisioning mode.
- [x] Use the core service/BOOT-button long press as a two-stage gesture: from normal operation, enter temporary non-destructive configuration; a new long press while in temporary configuration invokes the existing Wi-Fi credential reset through WiFiManager::configure(). Preserve the existing light-button gestures and the smart-display Reconfigure/Restore workflow.
- [x] Implement explicit core configuration mode valid only until the next user restart or power cycle, retaining credentials on first entry. Keep separate concepts for saved credentials, actual connectivity and configuration mode. Account for the Bluetooth memory-release lifecycle; if entry/reset needs an internal restart, carry a one-shot request into that boot and consume it immediately. Preserve smart-display persistent configuration alongside the temporary mode. The serial command `#CONFIG:wifi-setup` enters the same temporary flow without erasing credentials.
- [x] Consume each physical BOOT hold only once. Require release before accepting a second long press, including across an internal restart, so one prolonged hold cannot enter configuration and then erase credentials. Prefer performing the requested restart after button release.
- [x] Give BOOT 1 (WPS only) and BOOT 2 (full provisioning) distinct system-LED blink patterns. BOOT 2 must remain recognizable while the device stays in configuration mode; preserve the existing Wi-Fi connection-loss overlay. BOOT 1 uses 500 ms on/off; BOOT 2 uses two 100 ms pulses per second.
- [x] Put **Waves** first in the effect list, before **Solid**, and select it at startup. Keep all named FX.Style values unchanged.
- [x] Ensure the status LED always provides visible feedback for every active effect: either the current color or an indication of effect progression, including effects whose first pixel is dark. Preserve light-off and brightness behavior.
- [x] Preserve historical status signalling when Wi-Fi credentials are saved: fast disconnected flashing, including the first connection attempt, and brief connected pulses. Color-light limits connected pulses to its initial 20-second window. Ordinary startup without credentials has no network warning. Explicit setup keeps its distinct blink patterns and effect overlays.
- [ ] Revisit non-blocking Wi-Fi reconnection separately. Existing stored credentials with an unavailable router can currently cause waits in the shared device loop. This change is deferred from the initial offline-startup work, which does not require reconnection changes when no credentials exist.

Implemented on 2026-10-30. Host policy and production-renderer regressions passed; isolated color-light-c3 (espressif32 8.0.0) and color-light-d1-mini (espressif8266 2.6.3) full builds passed. The two changed display S3 translation units also compiled successfully; the earlier full display build hit local Windows object-directory errors, so this is not a complete display firmware link or hardware acceptance. These checks are separate from device acceptance.

- Maintainer confirmed on 2026-10-30: the first BOOT hold gives regular blinking, a separate second hold gives the double-pulse/reset indication, and the device operates normally with local controls without reconnecting to Wi-Fi after the reset.
- [x] Confirm credential preservation after interrupted BOOT 1 WPS on color-light C3.
- [ ] Complete any remaining Waves-first/effect-feedback and physical network loss/recovery checks for the release candidate.
- [x] Separate provisioning services from credential erasure: the first BOOT hold/release always selects BOOT 1 (WPS only, Bluetooth off), with or without credentials. A second separate hold/release clears only Wi-Fi and selects BOOT 2 (WPS + Bluetooth). A normal offline boot remains operational. Preserve display first-time BLE setup and persistent Reconfigure/Restore; hide Restore Config in temporary BOOT modes.
- [x] Integrate automatic WPS in BOOT 1 and BOOT 2: preserve previous credentials in BOOT 1, erase them when advancing to BOOT 2, and save credentials directly from the SDK after WPS success and restart normally after one second, without an association/IP wait. Event callbacks publish flags; the main loop performs persistence/restart. Legacy SDK-only passwords are preserved before a new attempt.
- [x] Maintainer confirmed color-light C3 WPS success/save/restart, reconnection, preserved credentials after interrupted WPS, Wi-Fi reset and Bluetooth provisioning.
- [x] Retry WPS after errors/timeouts with a nonblocking two-second pause, keeping setup active and credentials intact until successful pairing and storage. Host tests cover repeated failures/recovery on ESP32 and ESP8266.
- [ ] Complete remaining target-specific acceptance, including ESP8266 WPS/reset and repeated router-timeout cycles on the final candidate.
- [x] Maintainer confirmed Smart Display S3 configuration tests after the Bluetooth completion and temporary-screen fixes: Reconfigure completes into normal startup; temporary WPS does not show Restore Config.
- [x] Fix live LED-count changes by resizing effect/output buffers together between frames, preserving existing colors, clipping script ranges and rolling back on allocation failure. Host regressions and maintainer color-light device test passed.

## Build platform and C3 memory policy

- [x] Pin all eight C3 environments to espressif32@7.1.3 (Arduino 2.x), retaining current source features and Bluetooth provisioning. Other ESP32 targets keep the default espressif32@~8.0.0; ESP8266 remains on espressif8266@2.6.3. Keep shared compiler flags and accurate C3 SystemInfo platform metadata.
- Maintainer measured about 126 KB free internal DRAM on the C3 with 7.1.3 and Wi-Fi connected. An 8.0.0 build with Bluetooth excluded still had about 99.5 KB free. The memory regression is not specific to the recent BOOT changes. See [platform policy](docs/build-platforms.md) and the local diagnostic report under artifacts/memory-comparison/.
- [ ] Validate the complete mixed-platform build matrix for the next release, plus C3 BOOT/WPS/Bluetooth provisioning and JavaScript execution with the pinned platform.
- [ ] Revisit Arduino 3.x on C3 when measured runtime heap is sufficient. Track SDK initialization/cleanup and IRAM placement; do not assume the entire gap is a BLE leak. A no-Bluetooth build is a diagnostic only, not the production workaround.

## Future work

- [ ] Design Mini notifications interoperable with HomeGenie Panel, reviewing the existing delivery protocol before choosing the Mini API. This is future work and is not included in 1.2.68.
- [ ] Add explicit notification registration/unregistration with a bounded persistent recipient registry and defined handling of obsolete registrations.
- [ ] Expose notification sending to JavaScript scripts using the existing Panel-compatible delivery path. Design bounded queued delivery so network waits do not block the main device/event loop.
- [ ] Add Android's per-device "Receive notifications from this device" checkbox for Mini and HomeGenie Server. Registration must follow the user's explicit choice; Server's existing unregister endpoint alone does not implement Mini notifications.

## Deferred light API consistency

- [ ] Review optional `Control.Level` transition units and the Dimmer status callback that currently overrides the requested duration. Consider decimal seconds to match `Control.ColorHsb`, preserving motor behavior and verifying existing callers before changing the API. This work was explicitly deferred and is separate from the current consolidation.
