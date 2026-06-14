# color-light

A smart light device with addressable RGB LEDs.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-led-lights/)


## Firmware configuration

In addition to default system options the following configuration options are available:

| Key        | Description                 | Default                                 |
|------------|-----------------------------|-----------------------------------------|
| `leds-pin` | LED strip GPIO pin number   | -1 (-1=not used)                        |
| `leds-cnt` | Number of LEDs              | 1 (1 - 1024)                            |
| `leds-rws` | Number of matrix rows       | 1 (1 - 64)                              |
| `leds-vrt` | Matrix vertical wiring      | false (false=horizontal, true=vertical) |
| `leds-srp` | Serpentine cabling layout   | true (false=progressive, true=serp.)    |
| `leds-stb` | Start layout at bottom      | false (false=start top, true=bottom)    |
| `leds-str` | Start layout at right       | false (false=start left, true=right)    |
| `leds-spd` | Data transfer speed         | 0 (0=800kHz, 256=400kHz)                |
| `leds-mod` | Add a module for each LED   | false                                   |
| `stld-pin` | Status LED (RGB) pin        | -1 (-1=not used)                        |
| `stld-typ` | Status LED type             | RGB/RGBW order mask (see code for ref.) |
| `stld-spd` | Status LED speed            | 0 (0=800kHz, 256=400kHz)                |
| `btn1-pin` | Toggle button GPIO pin num. | -1 (-1=not used)                        |
| `btn2-pin` | Mode button GPIO pin num.   | -1 (-1=not used)                        |


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
