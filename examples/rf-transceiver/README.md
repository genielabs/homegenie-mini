# rf-transceiver

A smart bidirectional RF gateway device supporting generic 433/315 MHz remotes (RC-Switch / fixed-code / EV1527) and **X10 RF** home automation protocols.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/ir-rf-gateway/)

> **Note:** This firmware natively merges and replaces the legacy `x10-transceiver` example (which is now deprecated).

---

## Features

- **Multi-Protocol Support:**
  - Standard 433.92 MHz fixed-code and learning-code remotes/sockets (RC-Switch).
  - Bidirectional **X10 RF** (433.92 / 315 MHz) for remotes, security sensors, and powerline control (via RF-to-PLC transceivers such as Marmitek TM13 / X10 TM751).
- **Flexible Hardware Compatibility:**
  - Classic discrete modules (e.g., **SYN480R** receiver + **SYN115** transmitter).
  - Software-configurable **TI CC1101** SPI transceiver (frequency, output power, RX filter bandwidth).
- **HomeGenie Smart Home Integration:**
  - Dynamic X10 device mapping (`x10-devs`) with typed modules (`Switch`, `Light`, `Dimmer`, `Siren`, `Fan`, `Shutter`, `Motor`, `DoorLock`).
  - Full scheduling support (time/cron and event-driven automation triggers).

---

## Firmware configuration

In addition to default system options, the following configuration parameters are available:

### Status LED (RGB)

| Key        | Description          | Default                                 |
|------------|----------------------|-----------------------------------------|
| `stld-pin` | Status LED (RGB) pin | -1 (-1=not used)                        |
| `stld-typ` | Status LED type      | RGB/RGBW order mask (see code for ref.) |
| `stld-spd` | Status LED speed     | 0 (0=800kHz, 256=400kHz)                |

### RF Hardware Selection

| Key         | Description                                                      | Default |
|-------------|------------------------------------------------------------------|---------|
| `rfhw-type` | RF module type: `""` (SYN480R + SYN115) or `"cc1101"` (CC1101)   | `""`    |

### Discrete RF Modules (SYN480R / SYN115)

| Key        | Description                    | Default |
|------------|--------------------------------|---------|
| `rfrc-pin` | RF Receiver GPIO (SYN480R DATA)| 7       |
| `rftr-pin` | RF Transmitter GPIO (SYN115 DATA)| 5     |

### CC1101 Transceiver (SPI & Radio Configuration)

| Key          | Description                                                    | Default  |
|--------------|----------------------------------------------------------------|----------|
| `ccmod-rxtx` | CC1101 GDO0 Data GPIO (Bidirectional RX/TX)                    | 4        |
| `ccspi-cs`   | CC1101 SPI Chip Select (CS/SS) GPIO                            | 9        |
| `ccspi-clk`  | CC1101 SPI Clock (SCK) GPIO                                    | 3        |
| `ccspi-mosi` | CC1101 SPI MOSI GPIO                                           | 10       |
| `ccspi-miso` | CC1101 SPI MISO GPIO                                           | 2        |
| `cccfg-freq` | Carrier frequency in MHz (e.g. `433.92`, `868.35`, `315.00`)    | `433.92` |
| `cccfg-powr` | TX output power in dBm (`-30` to `10`)                         | `10`     |
| `cccfg-bndw` | RX channel filter bandwidth in kHz (`58.0` to `812.0`)         | `270.0`  |

### X10 Home Automation

| Key         | Description                                                                                     | Default |
|-------------|-------------------------------------------------------------------------------------------------|---------|
| `x10-devs`  | Configured X10 devices list (e.g. `A1=Light;A2=Dimmer;B1=Switch;B2=Shutter`)                   | `""`    |
| `x10-hcode` | Default House Code fallback (A-P) *(deprecated in favor of `x10-devs`)*                          | `""`    |

---

## Modules and Web API

In addition to the common [HomeGenie Mini Device API](https://homegenie.it/mini/1.2/programming/api), this firmware exposes the following modules and REST API endpoints:

### 1. Generic RF Module (`HomeAutomation.HomeGenie/RF`)

Used for capturing and sending raw RF packets (RC-Switch / fixed-code / EV1527).

* **Domain / Address:** `HomeAutomation.HomeGenie/RF`
* **Properties:**
  * `Receiver.RawData`: Captured RF signal data as a hexadecimal string.
* **Commands:**
  * `Control.SendRaw/<raw_hex_string>/<repeat_count>/<repeat_delay_ms>`

### 2. X10 Modules (`HomeAutomation.X10/<address>`)

Each device configured in `x10-devs` (e.g. `A1`, `B2`) is exposed as an individual HomeGenie module:

* **Domain / Address:** `HomeAutomation.X10/<HouseCode><Unit>` (e.g., `HomeAutomation.X10/A1`)
* **Properties:**
  * `Status.Level`: Current status level (`0` = Off, `1`–`100` = On / Dim level).
* **Commands:**
  * `Control.On` &mdash; Turns the device ON (sends X10 RF `ON` command).
  * `Control.Off` &mdash; Turns the device OFF (sends X10 RF `OFF` command).
  * `Control.Toggle` &mdash; Toggles the current state.
  * `Control.Level/<0-100>` &mdash; Sets the brightness level (sends X10 RF `DIM` / `BRIGHT` commands).

#### Quick HTTP Examples:
```bash
# Turn ON X10 module A1
curl "http://<device_ip>/api/HomeAutomation.X10/A1/Control.On"

# Turn OFF X10 module A1
curl "http://<device_ip>/api/HomeAutomation.X10/A1/Control.Off"

# Set Dimmer A2 to 50%
curl "http://<device_ip>/api/HomeAutomation.X10/A2/Control.Level/50"

# Send a raw 433 MHz RF packet
curl "http://<device_ip>/api/HomeAutomation.HomeGenie/RF/Control.SendRaw/C133E068/10/100"
```

---

### Manual build and install

You can also manually build and install the firmware from source code as explained in the [Getting started](https://homegenie.it/mini/1.2/getting-started#custom-firmware) page using the following PlatformIO command:

```bash
pio run -e rf-transceiver[<target>] -t upload
```

where the `<target>` suffix corresponds to your specific board:

- **ESP32 (Generic / NodeMCU)**  
  *(none)* &rarr; `pio run -e rf-transceiver -t upload`
- **ESP8266 (D1 Mini / NodeMCU)**  
  `-d1-mini` &rarr; `pio run -e rf-transceiver-d1-mini -t upload`
- **ESP32-C3 (Zero / Mini)**  
  `-c3` &rarr; `pio run -e rf-transceiver-c3 -t upload`
- **ESP32-S3 (Zero / Mini)**  
  `-s3` &rarr; `pio run -e rf-transceiver-s3 -t upload`
- **Arduino Nano ESP32 (S3)**  
  `-s3-nano` &rarr; `pio run -e rf-transceiver-s3-nano -t upload`
