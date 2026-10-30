# Local operation and configuration mode

Normal operation does not require saved Wi-Fi credentials. With credentials,
the device still attempts to connect using the existing reconnect logic.
Blocking reconnect waits are a separate, deferred change.

## BOOT/service button

Hold the configured service button for at least three seconds, then release it.
The indicator previews the requested action after the hold threshold; the action
is performed only after a debounced release.

| Current state | Long press and release | System LED |
| --- | --- | --- |
| Normal operation, saved credentials | BOOT 1: WPS only; keep credentials, Bluetooth off | 500 ms on, 500 ms off |
| Normal operation, no saved credentials | BOOT 2: WPS + Bluetooth; no credential erase needed | Two 100 ms pulses per second |
| Configuration | Reset stored Wi-Fi SSID/password and restart into BOOT 2 | Two 100 ms pulses per second |

The second action requires a new press after release. A button already held at
startup is ignored until released, so a prolonged hold cannot perform both actions.
Device settings such as LED GPIO, matrix layout/count, motor limits and scripts
are not cleared by the Wi-Fi credential reset.

Configuration entry uses a one-shot boot request, consumed before services start.
The next ordinary reboot or power cycle returns to normal operation, even when
no credentials have been saved (except the legacy smart-display first-time
configuration workflow described below). Saving credentials through the existing
configurator and restarting returns to normal network operation.

The optional serial command `#CONFIG:wifi-setup` requests the same non-destructive
temporary configuration entry. It also works when no service button is configured.
Use LF line endings, as for the other existing serial commands. It is idempotent
while configuration is already active. `#RESET` retains its ordinary reboot behavior;
`#WPS` retains the explicit WPS/reset operation on supported builds. During
temporary setup it restarts the BOOT 2 reset/provisioning flow.

Both BOOT 1 and BOOT 2 automatically start WPS on supported builds. BOOT 1 keeps
the previous credentials; a timeout, failed connection or ordinary reboot does
not replace them. Older WPS passwords stored only by the SDK are first copied
into the existing saved Wi-Fi configuration. Entering BOOT 2 from BOOT 1 erases
the old credentials. Entering BOOT 2 directly without credentials does not execute
a redundant reset. Mode selection never makes an ordinary offline boot enter setup.
Only after the newly paired network obtains an IP are its SSID and password
saved, followed by an automatic normal restart after one second. Configuration
blinking remains active while waiting; no serial command is needed for success.
SDK events only publish flags; the main loop handles WPS completion, persistence
and restart, keeping the BOOT button and configuration LED responsive.
Router/device WPS acceptance is pending; host tests simulate SDK outcomes.

Bluetooth provisioning is available only on boards/builds that support it.
Serial provisioning remains available. This change does not add Bluetooth or a
new Wi-Fi access-point portal to ESP8266/ESP32-S2 builds.

## System LED

- No network-status blinking merely because credentials are absent, or because
  the first connection has not yet been established.
- After a previously established Wi-Fi connection is lost, retain the existing
  100 ms on / 100 ms off indication until it reconnects.
- Configuration indicators take precedence over network-loss indication.
- Color-light retains the current effect color between overlay pulses. Sparse
  effects can use a lit pixel beyond pixel zero; dark frames fall back to the
  current selected light color. Light-off and matrix power limiting are preserved.

## Smart-display compatibility

Smart-display explicitly retains its existing first-time configuration screen,
Bluetooth provisioning and persistent Reconfigure/Restore behavior. The new
temporary BOOT/serial entry is available alongside that existing workflow.
The persistent display mode is not converted into a one-shot setting. Its
Reconfigure operation keeps saved credentials and enables Bluetooth; it is separate
from WPS-only BOOT 1. First-time setup without credentials still offers Bluetooth.
The physical BOOT gesture follows the common BOOT 1/BOOT 2 rules. During BOOT 1,
the display shows router-WPS instructions instead of the Bluetooth app QR code.

## Color-light hardware acceptance

Waves (`distortion_waves`) is the first listed effect, before Solid, and is selected
at startup. Power-on brightness
and the existing light-button gestures are unchanged. The existing pixel count,
16x16 layout, wiring direction and power limit are read from stored settings.

1. With the device already configured, turn on the light and exercise the effects
   and brightness controls. Verify status-LED feedback also for sparse effects.
2. Hold BOOT for three seconds and release: expect a restart and regular blinking.
   Restart again with `#RESET`: the original Wi-Fi connection must return.
3. Enter configuration again, release BOOT, then perform a separate long press:
   expect the two-pulse indication. Reboot: local light controls must work without
   Wi-Fi credentials and without disconnected-status blinking.
4. Without credentials, long-press BOOT once: expect BOOT 2 directly and Bluetooth
   advertising on supported boards. Provision through the existing app/site/serial
   workflow. Confirm that Wi-Fi and the stored matrix settings are retained.
5. With credentials, enter BOOT 1: verify that Bluetooth is not advertised and that
   WPS succeeds or an ordinary reboot restores the previous network.
6. Separately check network loss/recovery and the smart-display Reconfigure/Restore
   workflow. Initial connection/reconnect waits are not made asynchronous here.

Host tests validate policy and renderer behavior; they do not replace these
hardware checks or measure physical button timing, BLE provisioning or LED signals.
