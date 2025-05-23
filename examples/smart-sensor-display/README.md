
# smart-sensor-display

Smart multi-sensor device with display. HomeGenie Mini UI example application.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-sensor/)


## Firmware configuration

Besides the default system options and the *smart-sensor* options detailed in the `../smart-sensor/README.md` file,
the `smart-sensor-display` firmware offers the following additional configuration settings:

| Key         | Description                                                | Default  |
|-------------|------------------------------------------------------------|----------|
| `disp-typ`  | Display driver ("GC9A01" or "ST7789")                      | "GC9A01" |
| `disp-sclk` | DISPLAY_SCLK                                               |          |
| `disp-mosi` | DISPLAY_MOSI                                               |          |
| `disp-miso` | DISPLAY_MISO                                               |          |
| `disp-dc`   | DISPLAY_DC                                                 |          |
| `disp-cs`   | DISPLAY_CS                                                 |          |
| `disp-rst`  | DISPLAY_RST                                                |          |
| `disp-bl`   | DISPLAY_BL (backlight)                                     |          |
| `dstch-prt` | TOUCH_PORT                                                 |          |
| `dstch-adr` | TOUCH_ADDRESS                                              |          |
| `dstch-sda` | TOUCH_SDA                                                  |          |
| `dstch-scl` | TOUCH_SCL                                                  |          |
| `dstch-rst` | TOUCH_RST                                                  |          |
| `dstch-int` | TOUCH_INTERRUPT                                            |          |
| `imus-typ`  | Inertial Measurement Unit type ("QMI8658")                 |          |
| `imus-sda`  | IMU SDA                                                    |          |
| `imus-scl`  | IMU SCL                                                    |          |
| `motn-pms`  | If set to "sleep" enable deep-sleep bound to motion sensor |          |

For default GPIO# values used by the display driver see `ui/drivers/RoundDisplay.h` and
`ui/drivers/StandardDisplay.h` files.


## UI Activities configuration


| Key                  | Description                        | Default               |
|----------------------|------------------------------------|-----------------------|
| `title-<address>`    | Displayed title/name               |                       |
| `rcam-<address>`     | Remote camera JPEG images feed URL |                       |
| `rcam-res-<address>` | Camera resolution (1 to 17)        | "3" remote, "5" local |
| `rcam-qlt-<address>` | Camera resolution (10 to 63)       | "10"                  |

Where `<address>` is the address assigned to the associated Activity module
(e.g. `D1`, `M1`, `V1`...).  
These values can also be easily configured using the *HomeGenie Panel* app.



### Manual build and install

You can also manually build and install the firmware from source code
as explained in the [Getting started](../../getting-started#custom-firmware) page
and using the following commands for flashing the firmware:

```bash
pio run -e smart-sensor[<target>] -t upload
```

where the optional `<target>` suffix can be one of the following:
- ESP32 + display  
  `-display`
- ESP32-S3 + PSRAM + Display  
  `-display-s3`
- ESP32-S3 + OPI PSRAM + Display  
    `-display-s3-opi`
