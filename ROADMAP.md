# Roadmap

Status: 2026-10-22. This list separates implemented changes and observed results from pending release checks. See [CHANGELOG.md](CHANGELOG.md).

## Current consolidation

- [x] Implement ESP32 Arduino 3.x compatibility while preserving guarded older-core and ESP8266 paths. Initial validation covered all 45 environments; the maintainer subsequently confirmed complete successful builds after the final firmware changes.
- [x] Correct HTTP transport lifetime in JavaScript helpers. Maintainer observed about 8 h 10 min of motion-triggered operation after the fix; retain intermittent-crash investigation if new evidence appears.
- [x] Restore SSDP discovery, HTTP access and consistent device/API time; maintainer confirmed these on the test device.
- [x] Move eligible LVGL, sprite and Duktape allocations to PSRAM with fallback; maintainer confirmed improved DRAM availability and the tested display/script/camera workflow.
- [x] Restore concurrent display/HTTP frame sharing with last-reader release. Maintainer confirmed the final simultaneous-preview behavior after the serialized version's FPS regression.
- [x] Complete the final environment build check after the PSRAM/shared-camera and compatibility changes. The maintainer confirmed all builds succeed; verify the release CI output separately.
- [x] Complete maintainer camera acceptance. Local/remote viewing and the final shared-frame implementation were confirmed working, and the maintainer marked the camera check complete. Preserve frame sharing and valid buffer lifetime; continue investigating if new failures appear.
- [x] Resolve and verify the smart-motor S3 eight-output regression. The local ESP32Servo fixes passed native tests and maintainer hardware checks on the combined four-wheel/four-servo robot-car and arm. A remaining wheel issue was resolved by correcting its stored 0-80 angle range to 0-180. Website initialization presets were updated and tested; firmware boot does not indiscriminately overwrite user limits.
- [x] Observe motion-triggered scripts, Wi-Fi availability, uptime and DRAM under normal use; the maintainer marked this acceptance check complete. Keep future investigation driven by new evidence, without reviving the reverted event/scheduler locking experiments.
- [x] Verify website uploads on S3-CAM after cold USB reconnect, S3-Zero and C3-Zero/Mini. Classic ESP32-CAM upload works with manual IO0, as with PlatformIO; automatic entry on that tested board is not confirmed.
- [x] Set firmware version to 1.2.68 and align documentation with confirmed results.
- [ ] Commit the complete candidate, including the vendored ESP32Servo and new compatibility/memory helpers, then verify fresh CI builds and ZIP artifacts for tag 1.2.68. Existing local artifacts are not evidence of a completed release.

## Future work

- [ ] Design Mini notifications interoperable with HomeGenie Panel, reviewing the existing delivery protocol before choosing the Mini API. This is future work and is not included in 1.2.68.
- [ ] Add explicit notification registration/unregistration with a bounded persistent recipient registry and defined handling of obsolete registrations.
- [ ] Expose notification sending to JavaScript scripts using the existing Panel-compatible delivery path. Design bounded queued delivery so network waits do not block the main device/event loop.
- [ ] Add Android's per-device "Receive notifications from this device" checkbox for Mini and HomeGenie Server. Registration must follow the user's explicit choice; Server's existing unregister endpoint alone does not implement Mini notifications.
