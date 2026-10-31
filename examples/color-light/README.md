# color-light

A smart light device with addressable RGB LEDs.

- [Documentation and firmware install page](https://homegenie.it/smart-light/)


## Offline use and provisioning

Local light controls and effects work without configured Wi-Fi. Hold BOOT for
four seconds, then release: the first long press always enters BOOT 1 (WPS only),
even if no Wi-Fi credentials have been saved. The LED flashes slowly and regularly.
A second separate long press in temporary configuration resets Wi-Fi credentials
and enters BOOT 2, enabling Bluetooth with a repeated double flash. Ordinary offline startup
does not enable configuration or Bluetooth. See [configuration mode and device checks](../../docs/configuration-mode.md)
for indicator patterns, restart behavior and the 16x16 acceptance sequence.
`#CONFIG:wifi-setup` provides the same non-destructive entry via serial. A second
separate physical long press in configuration resets only Wi-Fi credentials,
not GPIOs, layout or other device settings. WPS starts on supported builds and
retries automatically after errors or timeouts. The device restarts only after
saving the credentials received from a successful WPS pairing. Wi-Fi connects
during normal startup; setup does not wait for an IP address.
Waves is the first/initial effect; the existing power-on state is preserved.

The website's Initialize only option applies the complete selected form, not
only Wi-Fi. Blank Wi-Fi fields preserve saved credentials. The public 1.2.68
release predates the new BOOT flow, matrix script fix and auxiliary C2 feature;
use updated firmware for these features.


## Firmware configuration

In addition to default system options the following configuration options are available:

| Key        | Description                 | Default                                 |
|------------|-----------------------------|-----------------------------------------|
| `leds-pin` | LED strip GPIO pin number   | -1 (-1=not used)                        |
| `leds-cnt` | Number of LEDs              | 1 (1 - 4096)                            |
| `leds-rws` | Number of matrix rows       | 1 (1 - 64)                              |
| `leds-srp` | Serpentine cabling layout   | true (false=progressive, true=serp.)    |
| `leds-vrt` | Matrix vertical wiring      | false (false=horizontal, true=vertical) |
| `leds-stb` | Start layout at bottom      | false (false=start top, true=bottom)    |
| `leds-str` | Start layout at right       | false (false=start left, true=right)    |
| `leds-pwr` | Matrix/strip maximum power percentage | 25 (5 - 100) |
| `leds-spd` | Data transfer speed         | 0 (0=800kHz, 256=400kHz)                |
| `leds-mod` | Add a module for each LED   | false                                   |
| `stld-pin` | Status LED (RGB) pin        | -1 (-1=not used)                        |
| `stld-typ` | Status LED type             | RGB/RGBW order mask (see code for ref.) |
| `stld-spd` | Status LED speed            | 0 (0=800kHz, 256=400kHz)                |
| `btn1-pin` | Toggle button GPIO pin num. | -1 (-1=not used)                        |
| `btn2-pin` | Mode button GPIO pin num.   | -1 (-1=not used)                        |


## Auxiliary controller LED (C2)

An optional addressable RGB/RGBW controller LED can mirror the status indicator
or display a separately controlled color. It is disabled by default and creates
no C2 module until a separate GPIO is configured. It does not require a board
with a built-in RGB LED.

| Key | Description | Default |
| --- | --- | --- |
| `auld-pin` | Auxiliary LED GPIO (`-1` disables it; restart required) | `-1` |
| `auld-typ` | NeoPixel color-order mask (restart required) | `82` |
| `auld-spd` | Data speed: `0` = 800 kHz, `256` = 400 kHz (restart required) | `0` |
| `auld-mod` | `status` or `independent` | `status` |

`status` mirrors the displayed effect color, system blink patterns and activity
indication. It also works without a physical built-in status LED. Existing
rules for offline startup, lost Wi-Fi and BOOT configuration remain unchanged.

`independent` displays C2's own on/off, level, color and transitions. It ignores
C1 effects and system blinking. C2 starts off at boot, and retains its independent
color while temporarily switched to status mode. Its `LED.mode` option in the
app changes the mode immediately and persists it as `auld-mod`. In status mode,
C2's properties continue to represent its independent control state; mirroring
never emits an event for every animation frame. C2 does not yet drive a second
strip/matrix output.

GPIOs shared with the main strip, built-in RGB indicator or system status LED
are rejected. Pin/type/speed changes take effect after restarting.

For the test controllers, set GPIO 8 on C3 or GPIO 9 on S3 (not global board
presets). Serial commands use a space between key and value:

```text
#SET:auld-pin 8
#SET:auld-typ 82
#SET:auld-spd 0
#SET:auld-mod status
#RESET
```

Use `#SET:auld-pin 9` instead on S3. After boot, check effect replication and
BOOT indication, then choose `independent` in C2's options and test its color,
level and transitions. C1 and the matrix should keep running unchanged.


## Scripting Engine & Automation

The firmware features an on-the-fly compiled bytecode scripting engine designed for creating custom and highly reactive LED animations.

### Script Syntax & Specifications

*   **Time Formats:** Timestamps can be fully expressed or abbreviated as `hh:mm:ss.xxx`, `mm:ss.xxx`, or simply in seconds as `ss.xxx` (where `.xxx` represents milliseconds).
*   **LED Range Selection:**
    *   `ALL`: Targets the entire matrix dynamically.
    *   `start-end` (1D): Targets a 1D linear physical range (e.g. `0-15`).
    *   `start+length` (1D): Targets a physical range starting at `start` and spanning `length` LEDs.
    *   `row:col` (2D): Targets a single pixel on a 2D coordinate space.
    *   `row:col+width` (2D): Targets a horizontal segment on a specific row.
    *   `row:col+width:height` (2D): Targets a rectangular area, automatically unrolled and clipped to the matrix boundaries.
*   **Supported Commands:**
    *   `On` / `Off` / `Toggle`: Classic state overrides.
    *   `Level(value[, transition])`: Sets brightness from 0 to 100.
    *   `HSB(h, s, b[, transition])`: Sets custom HSB color (floats from 0.0 to 1.0).
    *   `Default` / `Current` / `Default(value[, transition])`: Uses the master color currently set by the app (supports optional scaling and transition times).
    *   `#N` / `#LOOP:N` / `#REPEAT#N count`: Defines labels and controls infinite loops or repetitions.

### Running a Script (HTTP POST Example)

You can send a script directly to the device using a raw HTTP POST request with `Content-Type: text/plain`. 

Here is an example of a static Pac-Man ghost on a 16x16 matrix, featuring white eyes with black pupils and a body
that automatically adapts to the active master color:

```bash
curl -X POST http://<device_ip>/api/HomeAutomation.HomeGenie/Automation/Script.Run/ \
-H "Content-Type: text/plain" \
--data-binary @- <<'EOF'
00.000 ALL Off

#1
// 1. Ghost Body (Default Color using the ultra-compact 14x10 rectangle)
00.000 1:4+8 DEFAULT
00.000 2:2+12 DEFAULT
00.000 3:1+14:10 DEFAULT
00.000 13:1+3 DEFAULT
00.000 13:5+6 DEFAULT
00.000 13:12+3 DEFAULT
00.000 14:1 DEFAULT
00.000 14:5+2 DEFAULT
00.000 14:9+2 DEFAULT
00.000 14:14 DEFAULT

// 2. White of the eyes (Two compact 4x4 squares)
00.000 4:4+4:4 HSB(0,0,1,0)
00.000 4:10+4:4 HSB(0,0,1,0)

// 3. Black pupils (Two 2x2 squares, drawn on top of the whites)
00.000 6:5+2:2 Off
00.000 6:11+2:2 Off

#LOOP:1
EOF
```

## Manual build and install

You can manually build and install the firmware from source code
as explained in the [Getting started](https://homegenie.it/mini/1.2/getting-started#custom-firmware) page
and using the following command for flashing the firmware:

```bash
pio run -e color-light[<target>] -t upload
```

where the optional `<target>` suffix can be one of the following:
- ESP8266  
  `-d1-mini`
- ESP32 (generic)  
  *none*
- ESP32-C3  
  `-c3`
- ESP32-S3  
  `-s3`
- ESP32 D1 Mini    
  `-d1-mini-esp32`


## Credits & Inspirations

Several matrix effects in this firmware are custom procedural adaptations inspired by or adapted from popular open-source projects:
- **WLED**: Distortion Waves (by ldirko, blazoncek, and dedehai) and 1D Particle Fire (by Damian Schneider / DedeHai).
- **FastLED / Soulmate Lights**: 2D Drift (by Stepko, modified by Andrew Tuline) and 2D Polar Lights (by Kostyantyn Matviyevskyy, modified by Andrew Tuline).

### Matrix orientation and scripts

Two-dimensional script selections use the configured matrix mapping, including
vertical wiring, serpentine rows/columns and bottom/right origins. Coordinate
rectangles are compiled into contiguous physical spans along the wiring axis.
Explicit physical LED indices/ranges and `ALL` keep their existing behavior.
There is no extra frame buffer or per-frame coordinate conversion.

After changing the matrix layout in the app, resend the image/script so it is
compiled with the new mapping. Source scripts are deliberately not retained in
RAM after compilation. To check orientation, use the same layout that displays
Fire correctly, then send the Pac-Man Ghost again.

Host regression test (requires Python and a C++11 compiler):

```sh
python test/matrix-script/test.py c++
```

### Website configurator

The Smart Light form exposes all four matrix layout flags as Yes/No selects
with literal `true`/`false` values. It includes the optional `auld-*` section;
no board-wide auxiliary GPIO default is applied. The tested controller pins are
C3 GPIO 8 and S3 GPIO 9. Matrix LED count must be divisible by its row count.
The form validates dimensions and auxiliary/shared-LED GPIO conflicts before
sending configuration. All device parameters are sent with `#SET:key value`;
network credentials use `#CONFIG:key value` and are skipped when empty.

For the tested upright 16x16 mounting: 256 LEDs, 16 rows, vertical and serpentine
true, bottom true, right false. This is a mounting example, not a universal
panel default. Configure the layout first, then resend the coordinate script.

### Changing the number of LEDs

The app's LED count setting takes effect without restarting the device. The
render loop resizes the output and effect buffers together between frames.
Existing pixel colors are retained; scripts restart with ranges limited to the
new output size. Resend coordinate-based scripts after changing the matrix shape.
If memory is insufficient, the previous size and its saved setting are retained.
Individual LED handlers are reused across changes; handlers above a reduced LED
count remain registered until reboot and do not write to the output.
