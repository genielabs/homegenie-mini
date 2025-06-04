# x10-transceiver

A smart RF gateway device for X10 home automation protocol.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/ir-rf-gateway/)


## Firmware configuration

In addition to default system options the following configuration options are available:

| Key        | Description          | Default                                 |
|------------|----------------------|-----------------------------------------|
| `stld-pin` | Status LED (RGB) pin | -1 (-1=not used)                        |
| `stld-typ` | Status LED type      | RGB/RGBW order mask (see code for ref.) |
| `stld-spd` | Status LED speed     | 0 (0=800kHz, 256=400kHz)                |
| `rfrc-pin` | RF Receiver GPIO #   | 7                                       |
| `rftr-pin` | RF Transmitter GPIO# | 5                                       |


### Manual build and install

You can also manually build and install the firmware from source code
as explained in the [Getting started](https://homegenie.it/mini/1.2/getting-started#custom-firmware) page
and using the following commands for flashing the firmware:

```bash
pio run -e x10-transceiver[<target>] -t upload
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

### [HomeAutomation.X10](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html) API (x10-transceiver firmware)

Implemented subset:

- [`/api/HomeAutomation.X10/<module_address>/Control.On`](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html#1)
- [`/api/HomeAutomation.X10/<module_address>/Control.Off`](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html#2)
- [`/api/HomeAutomation.X10/<module_address>/Control.Level`](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html#5)
- [`/api/HomeAutomation.X10/<module_address>/Control.Toggle`](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html#6)

