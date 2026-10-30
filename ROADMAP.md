# Roadmap

Status: 2026-10-30. This list separates implemented changes and observed results from pending release checks. See [CHANGELOG.md](CHANGELOG.md).

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
- [x] Implement explicit core configuration mode valid only until the next user restart or power cycle, retaining credentials on first entry. Keep separate concepts for saved credentials, actual connectivity and configuration mode. Account for the Bluetooth memory-release lifecycle; if entry/reset needs an internal restart, carry a one-shot request into that boot and consume it immediately. Preserve smart-display persistent configuration alongside the temporary mode. An additional serial entry for the website configurator remains optional.
- [x] Consume each physical BOOT hold only once. Require release before accepting a second long press, including across an internal restart, so one prolonged hold cannot enter configuration and then erase credentials. Prefer performing the requested restart after button release.
- [x] Give BOOT 1 (WPS only) and BOOT 2 (full provisioning) distinct system-LED blink patterns. BOOT 2 must remain recognizable while the device stays in configuration mode; preserve the existing Wi-Fi connection-loss overlay. BOOT 1 uses 500 ms on/off; BOOT 2 uses two 100 ms pulses per second.
- [x] Put **Waves** first in the effect list, before **Solid**, and select it at startup. Keep all named FX.Style values unchanged.
- [x] Ensure the status LED always provides visible feedback for every active effect: either the current color or an indication of effect progression, including effects whose first pixel is dark. Preserve light-off and brightness behavior.
- [x] Blink the status LED only after an established Wi-Fi connection is lost, or while explicit configuration mode is active. Starting without saved credentials, or waiting for the first connection, must not by itself trigger disconnected-status blinking. Preserve the existing connection-loss overlay on the running effect without stopping or resetting that effect.
- [ ] Revisit non-blocking Wi-Fi reconnection separately. Existing stored credentials with an unavailable router can currently cause waits in the shared device loop. This change is deferred from the initial offline-startup work, which does not require reconnection changes when no credentials exist.

Implemented on 2026-10-30. Host policy and production-renderer regressions passed; isolated color-light-c3 (espressif32 8.0.0) and color-light-d1-mini (espressif8266 2.6.3) full builds passed. The two changed display S3 translation units also compiled successfully; the earlier full display build hit local Windows object-directory errors, so this is not a complete display firmware link or hardware acceptance. These checks are separate from device acceptance.

- Maintainer confirmed on 2026-10-30: the first BOOT hold gives regular blinking, a separate second hold gives the double-pulse/reset indication, and the device operates normally with local controls without reconnecting to Wi-Fi after the reset.
- [ ] Remaining smart-light acceptance: credential preservation on first entry, Waves-first ordering/default and status-LED feedback across effects.
- [x] Separate provisioning services from credential erasure: with saved credentials, explicit entry selects BOOT 1 (WPS only, Bluetooth off); without credentials, it selects BOOT 2 (WPS + Bluetooth) without a redundant erase. A normal offline boot remains operational. Preserve the display's first-time BLE setup and non-destructive persistent Reconfigure/Restore; show WPS instructions during physical BOOT 1.
- [x] Integrate automatic WPS in BOOT 1 and BOOT 2: preserve previous credentials in BOOT 1, erase them when advancing to BOOT 2, and save new credentials only after IP acquisition before an automatic normal restart after one second. Event callbacks publish flags; the main loop performs persistence/restart. Legacy SDK-only passwords are preserved before a new attempt.
- [ ] Hardware WPS acceptance: BOOT 1 failed/interrupted attempt followed by reboot restores the previous network; BOOT 2 failure leaves no saved network; Bluetooth is absent in BOOT 1 and available in BOOT 2 on supported boards; successful WPS automatically restarts and reconnects with the new credentials. Verify on ESP32 and ESP8266.
- [ ] Maintainer acceptance of smart-display first-time provisioning and persistent Reconfigure/Restore alongside temporary configuration.

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
