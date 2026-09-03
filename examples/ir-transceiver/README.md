# ir-transceiver

A smart Infrared (IR) transceiver gateway device for capturing and replaying 38 kHz remote control signals (TVs, air conditioners, set-top boxes, and audio systems).

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/ir-rf-gateway/)

---

## Features

- **Infrared Control:**
  - 38 kHz Infrared signal capture with raw timing decoding.
  - Signal replay with customizable repeat counts and repeat intervals.
- **HomeGenie Smart Home Integration:**
  - Full integration with HomeGenie Server, HomeGenie Panel apps, WebSockets, and REST API.
  - Event-driven automation and scheduling support.

---

## Firmware configuration

In addition to default system options, the following configuration parameters are available:

### Status LED (RGB)

| Key        | Description          | Default                                 |
|------------|----------------------|-----------------------------------------|
| `stld-pin` | Status LED (RGB) pin | -1 (-1=not used)                        |
| `stld-typ` | Status LED type      | RGB/RGBW order mask (see code for ref.) |
| `stld-spd` | Status LED speed     | 0 (0=800kHz, 256=400kHz)                |

### IR Module

| Key        | Description              | Default |
|------------|--------------------------|---------|
| `irrc-pin` | IR Receiver GPIO pin     | 7       |
| `irtr-pin` | IR Transmitter GPIO pin  | 5       |

---

## Modules and Web API

In addition to the common [HomeGenie Mini Device API](https://homegenie.it/mini/1.2/programming/api), this firmware exposes the following module and REST API endpoints:

### IR Module (`HomeAutomation.HomeGenie/IR`)

Used for capturing and transmitting raw 38 kHz Infrared signals.

* **Domain / Address:** `HomeAutomation.HomeGenie/IR`
* **Properties:**
  * `Receiver.RawData`: Captured IR signal data as a hexadecimal string.
* **Commands:**
  * `Control.SendRaw/<raw_hex_string>/<repeat_count>/<repeat_delay_ms>`

#### Quick HTTP Examples:
```bash
# Send an IR raw signal (e.g. TV Power) with 1 repeat and 100ms delay
curl "http://<device_ip>/api/HomeAutomation.HomeGenie/IR/Control.SendRaw/0000006D.../1/100"

# Query the last captured IR raw data
curl "http://<device_ip>/api/HomeAutomation.HomeGenie/IR/Receiver.RawData"
```

---

### Manual build and install

You can also manually build and install the firmware from source code as explained in the [Getting started](https://homegenie.it/mini/1.2/getting-started#custom-firmware) page using the following PlatformIO command:

```bash
pio run -e ir-transceiver[<target>] -t upload
```

where the `<target>` suffix corresponds to your specific board:

- **ESP32 (Generic / NodeMCU)**  
  *(none)* &rarr; `pio run -e ir-transceiver -t upload`
- **ESP8266 (D1 Mini / NodeMCU)**  
  `-d1-mini` &rarr; `pio run -e ir-transceiver-d1-mini -t upload`
- **ESP32-C3 (Zero / Mini)**  
  `-c3` &rarr; `pio run -e ir-transceiver-c3 -t upload`
- **ESP32-S3 (Zero / Mini)**  
  `-s3` &rarr; `pio run -e ir-transceiver-s3 -t upload`
- **Arduino Nano ESP32 (S3)**  
  `-s3-nano` &rarr; `pio run -e ir-transceiver-s3-nano -t upload`
