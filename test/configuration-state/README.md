# Configuration and status indicator host tests

From the repository root, using a C++11 compiler:

```sh
c++ -std=c++11 -Wall -Wextra -Werror test/configuration-state/test.cpp -o /tmp/mini-configuration-test
/tmp/mini-configuration-test
python3 test/configuration-state/render-test.py c++
python3 test/configuration-state/core-test.py c++
python3 test/configuration-state/wps-test.py c++
```

The first test exercises the actual common policy header: one-shot requests,
storage failures, long press/release, startup-held BOOT, debounce, timer rollover,
normal offline/online operation, connection loss and configuration LED patterns.
The renderer test extracts the production renderPixels function and compiles it
with pixel/color test doubles. It checks a 16x16 sparse frame, current-color
fallback, off, matrix power limiting, per-pixel overrides and a status-only LED.

The core test compiles the production Config/WiFiManager methods and Bluetooth
startup with in-memory preferences and a simulated restart. It verifies that temporary entry preserves
credentials, a second-stage reset clears only Wi-Fi credentials, matrix settings
survive, ordinary reboot exits temporary mode, display legacy modes remain
available and storage failures do not cause a restart. It also checks BOOT 1
Bluetooth suppression and memory release, direct BOOT 2 entry without credentials or erasure, normal
offline reboot and the display's preserved first-time/persistent Bluetooth path.

No hardware, flash storage or network traffic is involved. Embedded builds and
maintainer device checks remain separate. See ../../docs/configuration-mode.md.

The WPS test compiles the production provisioning methods for both platform
branches with deterministic SDK events. It covers success and delayed restart,
stale IP events, failed/timed-out pairing, connection timeout, legacy password
preservation, erased credentials and failed storage operations. It does not
validate the radio/SDK itself; run the embedded builds and device WPS checks.
