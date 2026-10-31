# Local operation and configuration mode

Normal operation does not require saved Wi-Fi credentials. With credentials,
the device still attempts to connect using the existing reconnect logic.
Blocking reconnect waits are a separate, deferred change.

## BOOT/service button

Hold the configured service button for at least four seconds, then release it.
The indicator previews the requested action after the hold threshold; the action
is performed only after a debounced release.

| Current state | Long press and release | System LED |
| --- | --- | --- |
| First press, with or without saved credentials | BOOT 1: WPS only; keep credentials, Bluetooth off | 500 ms on, 500 ms off |
| Another press in temporary configuration | Reset stored Wi-Fi SSID/password and restart into BOOT 2; Bluetooth enabled | Two 100 ms pulses per second |

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
the old credentials. The first press always selects BOOT 1, including when the
device has no credentials. Mode selection never makes an ordinary offline boot enter setup.
On WPS success, the loop reads the received SSID/password directly from the
station SDK configuration, verifies their storage and schedules a normal restart
after one second. It does not wait for association or an IP address. Normal
startup connects using the saved credentials through the existing Wi-Fi path. Configuration
blinking remains active while waiting; no serial command is needed for success.
After pairing, startup or credential-saving errors/timeouts, the loop
stops the previous attempt and automatically starts another after a nonblocking
two-second pause. Each retry clears old event flags and connection state. It does
not reboot, leave the selected mode, enable Bluetooth in BOOT 1 or replace the
previous credentials. Attempts continue until success, reset or power-off.
SDK events only publish flags; the main loop handles WPS completion, persistence
and restart, keeping the BOOT button and configuration LED responsive.
On 2026-10-31, the maintainer confirmed final WPS pairing/save/restart,
reconnection, credential preservation after interrupted WPS and Bluetooth
reset/provisioning on color-light C3. Smart Display S3 configuration tests also
passed after the Reconfigure completion and Restore Config visibility fixes.
Repeated timeout cycles and other target profiles remain separate checks;
host tests simulate SDK outcomes.

Bluetooth provisioning is available only on boards/builds that support it.
Serial provisioning remains available. This change does not add Bluetooth or a
new Wi-Fi access-point portal to ESP8266/ESP32-S2 builds.

## System LED

- Without saved Wi-Fi credentials, normal offline operation has no network warning.
- Preserve the historical core signal: disconnected = 100 ms on / 100 ms off;
  connected = 50 ms on after 1950 ms off (approximately one pulse every two seconds).
- Color-light displays the connected pulses only during its initial 20-second
  startup window. Later disconnects still show the fast signal until reconnected;
  a reconnection does not start another timed confirmation window.
- Explicit configuration indicators and held-button previews remain visible even
  after the startup window. They take precedence over normal network signals.
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
The first physical BOOT gesture always enters BOOT 1, even from guided first-time
or persistent display setup. A second gesture enters BOOT 2. During BOOT 1,
the display shows router-WPS instructions instead of the Bluetooth app QR code.

## Color-light hardware acceptance

Waves (`distortion_waves`) is the first listed effect, before Solid, and is selected
at startup. Power-on brightness
and the existing light-button gestures are unchanged. The existing pixel count,
16x16 layout, wiring direction and power limit are read from stored settings.

1. With the device already configured, turn on the light and exercise the effects
   and brightness controls. Verify status-LED feedback also for sparse effects.
2. Hold BOOT for four seconds and release: expect a restart and regular blinking.
   Restart again with `#RESET`: the original Wi-Fi connection must return.
3. Enter configuration again, release BOOT, then perform a separate long press:
   expect the two-pulse indication. Reboot: local light controls must work without
   Wi-Fi credentials and without disconnected-status blinking.
4. Without credentials, hold BOOT for four seconds and release: expect WPS-only
   BOOT 1 and no Bluetooth advertising. Repeat the gesture to enter BOOT 2, then
   provision through the app. Confirm Wi-Fi connection and retained matrix settings.
5. With credentials, enter BOOT 1: verify that Bluetooth is not advertised and that
   WPS succeeds or an ordinary reboot restores the previous network.
6. Separately check network loss/recovery and the smart-display Reconfigure/Restore
   workflow. Initial connection/reconnect waits are not made asynchronous here.

Host tests validate policy and renderer behavior; they do not replace these
hardware checks or measure physical button timing, BLE provisioning or LED signals.

## Cross-device acceptance: WPS first, Bluetooth second

Use the following sequence on an ESP32-C3 color-light and an ESP32-S3 device
(for example ir-rf-transceiver or smart-motor). Record the firmware environment
and whether each step passes. On smart-motor, keep the wheels/arm clear during restarts.

1. **Cancel without changing a saved network.** Start with working Wi-Fi. Hold
   BOOT for four seconds, release, and expect regular slow flashing. Bluetooth
   discovery must be off. Do not start router WPS. Power-cycle the device:
   the saved network and all device settings must still work.
2. **Choose Bluetooth.** Start setup again, release the button, then hold it for
   another four seconds and release. Expect repeated double flashes and app
   discovery over Bluetooth. Complete setup in HomeGenie Panel and verify the
   normal restart, Wi-Fi connection and retained pins/layout/scripts.
3. **Start without saved Wi-Fi.** Repeat the two presses to clear Wi-Fi, then
   power-cycle before using the app. On ordinary devices, local controls must
   work offline and Bluetooth must be off. Hold BOOT once: slow WPS flashing,
   still no Bluetooth. Repeat the gesture: double flashes and Bluetooth discovery.
4. **First-time WPS.** With no saved Wi-Fi, enter WPS with one press, activate WPS
   on the router and wait. Expect an automatic restart and a working Wi-Fi
   connection. No app initialization should be necessary.
5. **Retry and timeout.** Leave WPS waiting without activating the router for
   at least two attempts. Expect a retry message and a new WPS attempt after each
   timeout, with the same configuration blink and no reboot. Then activate router
   WPS and verify successful configuration and automatic restart. Also verify the button remains usable: another four-second press must still start
   Bluetooth setup. Leaving either temporary mode by power-cycling must not
   keep the device in that mode.
6. **Hold/release safety.** Hold the button for eight seconds continuously.
   Only the first action may happen when it is released. A separate press and
   release is required for Bluetooth. Also check a short press does not enter setup.
7. **Normal network indicators.** Once configured, disconnect and reconnect the
   network. Expect the historical fast disconnected signal and normal recovery.
   On color-light, connected pulses stop after the initial 20-second window;
   the current effect and optional status-replica auxiliary LED must remain correct.

### Smart Display

Keep the existing first-time screen, Reconfigure and Restore checks separate
from the physical button. Try the physical sequence with and without saved Wi-Fi,
including from the first-time screen: the first gesture must show WPS instructions
and no Bluetooth discovery; the second must show Bluetooth setup. Verify that
on-screen Reconfigure still preserves credentials and enables its existing setup,
and that Restore retains its previous behavior.

### Camera and ESP8266

On an ESP32-S3 camera build with Bluetooth, repeat the common sequence and verify
that camera access works after setup. On ESP32-CAM or ESP8266 builds without
Bluetooth, check first-press WPS, both distinct LED signals, power-cycle exit and
credential reset without expecting Bluetooth discovery. WPS must still be usable
for an unconfigured device on those builds. Verify image/IR/RF/light functions as
appropriate after returning to normal operation.

The website is intentionally not updated with this new sequence yet.

During temporary BOOT 1/BOOT 2 setup, Smart Display does not show Restore Config.
Power cycling exits that session. Restore Config remains available when setup
was entered through the persistent on-screen Reconfigure action.
