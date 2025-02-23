# smart-sensor

A smart multi-sensor device.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-sensor/)


## Firmware configuration

In addition to default system options the following configuration options are available:

| Key        | Description                           | Default                                 |
|------------|---------------------------------------|-----------------------------------------|
| `stld-pin` | Status LED (RGB) pin                  | -1 (-1=not used)                        |
| `stld-typ` | Status LED type                       | RGB/RGBW order mask (see code for ref.) |
| `stld-spd` | Status LED speed                      | 0 (0=800kHz, 256=400kHz)                |
| `ligh-typ` | Light sensor type                     | -1                                      |
| `ligh-pin` | Light sensor GPIO#                    | -1                                      |
| `motn-typ` | Motion sensor type                    | -1                                      |
| `motn-pin` | Motion sensor GPIO#                   | -1                                      |
| `soth-typ` | Temperature sensor type               | -1                                      |
| `soth-pin` | Temperature sensor GPIO#              | -1                                      |
| `sdht-typ` | Temperature + Humidity sensor type    | -1                                      |
| `sdht-pin` | Temperature + Humidity sensor GPIO#   | -1                                      |
| `sdht-adj` | Adjust temp. read value (-10 to +10)  | 0                                       |
| `colr-typ` | Color sensor type                     | -1                                      |
| `colr-sda` | Color sensor SDA pin                  | -1                                      |
| `colr-scl` | Color sensor SCL pin                  | -1                                      |


### Manual build and install

You can also manually build and install the firmware from source code
as explained in the [Getting started](../../getting-started#custom-firmware) page
and using the following commands for flashing the firmware:

```bash
pio run -e smart-sensor[<target>] -t upload
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
