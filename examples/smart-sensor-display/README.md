
# smart-sensor-display

Smart multi-sensor device with display. HomeGenie Mini UI example application.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-display/)


## Firmware configuration

Besides the default system options and the *smart-sensor* options detailed in the `../smart-sensor/README.md` file,
the `smart-sensor-display` firmware offers the following additional configuration settings:

| Key         | Description                                              | Default  |
|-------------|----------------------------------------------------------|----------|
| `disp-typ`  | Display driver ("GC9A01", "ST7789", "ST7796" or "AUTO")  | "GC9A01" |
| `disp-sclk` | DISPLAY_SCLK                                             |          |
| `disp-mosi` | DISPLAY_MOSI                                             |          |
| `disp-miso` | DISPLAY_MISO                                             |          |
| `disp-dc`   | DISPLAY_DC                                               |          |
| `disp-cs`   | DISPLAY_CS                                               |          |
| `disp-rst`  | DISPLAY_RST                                              |          |
| `disp-bl`   | DISPLAY_BL (backlight)                                   |          |
| `dstch-prt` | TOUCH_PORT                                               |          |
| `dstch-adr` | TOUCH_ADDRESS                                            |          |
| `dstch-sda` | TOUCH_SDA                                                |          |
| `dstch-scl` | TOUCH_SCL                                                |          |
| `dstch-rst` | TOUCH_RST                                                |          |
| `dstch-int` | TOUCH_INTERRUPT                                          |          |
| `imus-typ`  | Inertial Measurement Unit type ("QMI8658")               |          |
| `imus-sda`  | IMU SDA                                                  |          |
| `imus-scl`  | IMU SCL                                                  |          |

For default GPIO# values used by the display driver see `ui/drivers/RoundDisplay.h` and
`ui/drivers/StandardDisplay.h` files.


### Display configuration


| Key          | Description                    | Default             |
|--------------|--------------------------------|---------------------|
| `disp-bri`   | Set the display brightness     | 64 (range 1 - 127)  |
| `dsbrd-ssts` | Screen saver timeout (seconds) | 15 (0 = disabled)   |


### Configuring activities

To set the activities that will be available on the smart display UI use the `dashboard` option:

```
#SET:dashboard SensorValues,CameraDisplay:V1,CameraDisplay:V2,LevelControl:M1,ColorControl:H1,ColorControl:H2,DigitalClock
```

which is basically the comma-separated list of activity names followed eventually by a `:` to specify the module address
or other options where applicable.

To configure the title of an activity:

| Key                  | Description                        | Default           |
|----------------------|------------------------------------|-------------------|
| `title-<address>`    | Displayed title/name               | "&lt;address&gt;" |

Where `<address>` is the address assigned to the associated Activity module
(e.g. `D1`, `M1`, `V1`...).  
These values can also be easily configured using the *HomeGenie Panel* app.

Options available for `CameraDisplayActivity` configuration:

| Key                  | Description                        | Default               |
|----------------------|------------------------------------|-----------------------|
| `title-<address>`    | Displayed title/name               | "&lt;address&gt;"     |
| `rcam-<address>`     | Remote camera JPEG images feed URL |                       |
| `rcam-res-<address>` | Camera resolution (1 to 17)        | "3" remote, "5" local |
| `rcam-qlt-<address>` | Camera resolution (10 to 63)       | "10"                  |


## Manual build and install

You can also manually build and install the firmware from source code
as explained in the [Getting started](https://homegenie.it/mini/1.2/getting-started#custom-firmware) page
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
