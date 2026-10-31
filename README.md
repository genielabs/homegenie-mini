![homegenie-mini workflow](https://github.com/genielabs/homegenie-mini/actions/workflows/platformio.yml/badge.svg)

# HomeGenie Mini v1.2 <small>`ESP32 / ESP8266`</small>

HomeGenie Mini is an **open source library** for building custom firmware for smart devices based on *ESP32* or *ESP8266* SoC. It allows you to create fully independent smart devices with local logic, cloud-free operation, and built-in JavaScript automation.

https://homegenie.it/mini

<p align="center">
  <a href="https://deepwiki.com/genielabs/homegenie-mini"><img src="https://deepwiki.com/badge.svg" alt="Ask DeepWiki"></a>
</p>

## Features

- **Easy web-based installation** directly from your browser (via Web Serial API)
- Easy device configuration using Wi-Fi protected setup button (WPS) or Bluetooth
- Does not require an Internet connection to be configured or to work properly
- Time synchronization using internal RTC (ESP32), mobile app time or NTP
- Integrated actions scheduler supporting *extended Cron expressions* and JavaScript
- Device discovery via SSDP/UPnP with customizable device name
- Multichannel I/O: HTTP, WebSocket, SSE, MQTT, Serial
- Built-in status LED logic and GPIO control API
- Multi-threading support, event router, extensible API
- Direct connection to *HomeGenie Panel* (Wi-Fi AP, hotspot or local network)
- Seamless integration with *HomeGenie Server* and other services via MQTT

---

## Installation & Flashing

Depending on your preference, you can flash the firmware either directly from your browser or compile it manually using the CLI.

### Method 1: Web Installer (Recommended & Code-Free)
If you prefer a quick, visual setup without installing coding tools, you can upload pre-compiled firmwares directly from the browser:
1. Connect your ESP device to your computer via USB.
2. Open the [HomeGenie Mini Getting Started Page](https://homegenie.it/mini/1.2/getting-started/).
3. Use the **Firmware Upload form** at the bottom of the page: select your SoC model, enter your Wi-Fi credentials (optional), and click **Create device**.

### Method 2: PlatformIO CLI (For Developers)
To customize and compile the firmware from source code, install [Platform.IO core](https://docs.platformio.org/en/latest/installation.html) and download the [HomeGenie Mini source code](https://github.com/genielabs/homegenie-mini/releases).

Open a terminal in the project directory and run the following commands to install the required dependencies:

```bash
pio pkg update
pio pkg install
```

To build and upload the default firmware to your connected ESP device, run:

```bash
pio run -e default -t upload
```

---

## Configuration Environments

The `-e default` option builds the base firmware for a generic *ESP32* board.

To list all available configurations (including specific profiles for target devices), run:

```bash
pio project config
```

You can customize environments or define your own pin mappings and features by editing the `platformio.ini` file.

Build platforms are selected by target to preserve internal RAM for automation:

| Target | Platform | Arduino core |
| --- | --- | --- |
| ESP32-C3 (all C3 environments) | `espressif32@7.1.3` | 2.x |
| Other ESP32 targets, including S2/S3/C6 | `espressif32@~8.0.0` | 3.x |
| ESP8266 | `espressif8266@2.6.3` | Existing ESP8266 core |

The C3 exception retains the current firmware features, including Bluetooth
provisioning, while preserving JavaScript heap headroom. See
[build platform policy and memory evidence](docs/build-platforms.md).


---

## Pre-built Firmware Projects (Examples)

HomeGenie Mini provides several specialized configurations and ready-to-use firmware templates. You can find detailed guides, 3D printing files, and schematics on the official website:

- 📺 [**Smart Display & Control**](https://homegenie.it/mini/1.2/examples/smart-display/): Transforms an ESP32 with a TFT touch display into a local smart dashboard using LVGL or LovyanGFX.
- 🤖 [**Smart Car & Robotic Arm**](https://homegenie.it/robot-car/): Configures multi-servo motor control (`smart-motor` firmware) for DIY robotics and Mecanum wheels.
- 📷 [**Smart Camera + A.I.**](https://homegenie.it/mini/1.2/examples/smart-camera/): Implements video streaming and object tracking logic using ESP32-CAM models.
- 💡 [**Smart Light / Mood Lamp**](https://homegenie.it/smart-light/): Controls WS281x RGB LED strips with automated transitions, schedules, and custom animations.
- 🌡️ [**Smart Sensor**](https://homegenie.it/mini/1.2/examples/smart-sensor/): Gathers data from DHT22, LDR, and motion sensors with low overhead.
- 📟 [**Remote Control & Gateway**](https://homegenie.it/firefly/): A multi-protocol transceiver gateway (Firefly Kit) to capture, replay, and bridge IR, generic 433/315 MHz RF (SYN modules or CC1101 SPI), and bidirectional **X10 RF** home automation devices.

---

Smart Light also supports matrix orientation settings and an optional auxiliary
controller LED. The LED can follow the status LED or be controlled separately
through C2. See the [color-light README](examples/color-light/README.md) for wiring
and configuration.

## Connecting the Device

You can use Smart Light's buttons and effects without Wi-Fi.

### Start Wi-Fi setup

Hold the **BOOT button for 4 seconds, then release it**. On some boards, this
button is labeled B or IO0.

The first press always starts **WPS**, whether Wi-Fi has been configured before
or not. Press the WPS button on your router to connect. The LED flashes
**slowly and regularly**. Saved Wi-Fi settings are kept.

To erase saved Wi-Fi settings and configure the device through Bluetooth, hold
the **same button again for 4 seconds, then release it**. The device restarts,
ready to be found through Bluetooth in HomeGenie Panel. The LED shows **two quick
flashes, repeated**. Follow the app instructions to enter your network name and
password. Your other settings are kept.

WPS retries automatically after an error or timeout until you leave setup.
When WPS pairing succeeds, the device saves the new Wi-Fi settings and
restarts automatically. To leave setup without making changes, disconnect and
reconnect the power.

See [Device setup](https://homegenie.it/mini/1.2/device-setup/) for the complete
instructions, or [configuration mode](docs/configuration-mode.md) for core behavior
and device checks. Smart Display retains its on-screen setup options.

### Configure with a USB cable

In the web installer, select your device model, check the settings and enter the
network name and password. Select **Initialize only** and click **Create device**
to apply the settings without reinstalling the firmware.

All selected settings in the form will be applied. Leave the Wi-Fi fields empty
to keep the saved network name and password.

### Add the device to HomeGenie Panel

Open [HomeGenie Panel](https://play.google.com/store/apps/details?id=com.glabs.homegenieplus),
search for devices and select your device to add it to the dashboard.

![HomeGenie Panel - Discovery: select device](data/images/phone/hg_panel_discovery_select.png)

### Connecting to HomeGenie Server

HomeGenie Mini devices feature zero-configuration network discovery and seamless integration with [HomeGenie Server](https://github.com/genielabs/HomeGenie):

1. **Automatic Discovery (UPnP/SSDP):**  
   Once connected to your local Wi-Fi network, your HomeGenie Mini device is automatically detected by HomeGenie Server and appears as a device card in the **UPnP** dashboard group.

2. **One-Click MQTT Connection:**  
   Click the **`CONNECT`** button on the device card. HomeGenie Server will automatically provision and establish the secure MQTT communication link in the background&mdash;no manual broker configuration required.

3. **Browse Modules & Add Dashboard Widgets:**  
   Once connected, click the **`MODULES`** button to view all hardware endpoints exposed by the device (e.g., *IR Transceiver*, *RC RF*, *Color Light*, *X10 Devices*). From the popup dialog, you can select any module and add its interactive widget directly to your preferred dashboard groups.

---

## Monitoring and Serial Commands

To monitor logs and interact with the device via the serial interface, connect your device to your USB port and run:

```bash
pio device monitor -b 115200
```

### Serial Configuration Commands

You can configure the device or query its status directly from the serial terminal. Enter system commands prefixed by `#` or use `/api/` prefix for API commands.

#### Core Commands
- `#CONFIG:device-name <name>`
- `#CONFIG:wifi-ssid <ssid>`
- `#CONFIG:wifi-password <passwd>`
- `#CONFIG:wifi-setup` (start Wi-Fi setup without clearing saved Wi-Fi settings)
- `#VERSION` / `#RESET` / `#UPTIME` / `#WPS`

The legacy `#WPS` command clears saved Wi-Fi settings. Use `#CONFIG:wifi-setup`
to keep them.

#### Get/Set Parameters
- `#GET:<parameter_key>`
- `#SET:<parameter_key> <value>`

| Key        | Description                                    | Default          |
|------------|------------------------------------------------|------------------|
| `sys-rb-n` | BOOT / Wi-Fi setup button GPIO#                     | -1 (disabled)    |
| `sys-sl-n` | System status LED GPIO#                        | -1 (disabled)    |
| `io-typ01` | I/O Ch.1 type ('Switch', 'Light', 'Dimmer')     |                  |
| `io-pin01` | I/O Ch.1 GPIO#                                 | -1 (disabled)    |

*Example setting the status LED and BOOT button:*
```text
#SET:sys-sl-n 10
#SET:sys-rb-n 0
#RESET
```

---

# Disclaimer

<small><code>
THIS PROJECT IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS PROJECT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
</code></small>