
# smart-sensor-display

Smart multi-sensor device with display. HomeGenie Mini UI example application.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-display/)


## Wi-Fi setup

For first-time setup, follow the instructions on the display to connect through
Bluetooth with HomeGenie Panel.

To change Wi-Fi from the display, select **Reconfigure** and complete setup in
the app. The device saves the new credentials and restarts normally. **Restore
Config** leaves Reconfigure without replacing the saved Wi-Fi settings.

You can also use the BOOT button:

- Hold it for **4 seconds and release** to start WPS. The LED flashes slowly;
  press the WPS button on your router. Saved credentials are kept.
- Hold and release it again for **4 seconds** to clear Wi-Fi credentials and
  enable Bluetooth setup. The LED shows two quick flashes, repeated. Display
  and other device settings are kept.

WPS retries after errors or timeouts. Successful pairing saves the received
credentials and restarts the device; normal startup connects to Wi-Fi.
Temporary button setup ends on an ordinary restart or power cycle and does not
show Restore Config. That button belongs to the on-screen Reconfigure workflow.

See [configuration modes and device checks](../../docs/configuration-mode.md).

## Firmware configuration

Besides the default system options and the *smart-sensor* options detailed in the `../smart-sensor/README.md` file,
the `smart-sensor-display` firmware offers the following additional configuration settings:

| Key         | Description                                     | Default |
|-------------|-------------------------------------------------|---------|
| `disp-typ`  | Display driver (GC9A01, ST7789, ST7796 or AUTO) |         |
| `disp-sclk` | DISPLAY_SCLK                                    |         |
| `disp-mosi` | DISPLAY_MOSI                                    |         |
| `disp-miso` | DISPLAY_MISO                                    |         |
| `disp-dc`   | DISPLAY_DC                                      |         |
| `disp-cs`   | DISPLAY_CS                                      |         |
| `disp-rst`  | DISPLAY_RST                                     |         |
| `disp-bl`   | DISPLAY_BL (backlight)                          |         |
| `dstch-prt` | TOUCH_PORT                                      |         |
| `dstch-adr` | TOUCH_ADDRESS                                   |         |
| `dstch-sda` | TOUCH_SDA                                       |         |
| `dstch-scl` | TOUCH_SCL                                       |         |
| `dstch-rst` | TOUCH_RST                                       |         |
| `dstch-int` | TOUCH_INTERRUPT                                 |         |
| `imus-typ`  | Inertial Measurement Unit type (QMI8658)        |         |
| `imus-sda`  | IMU SDA                                         |         |
| `imus-scl`  | IMU SCL                                         |         |

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

| Key                  | Description                        | Default      |
|----------------------|------------------------------------|--------------|
| `title-<address>`    | Displayed title/name               | `<address>`  |

Where `<address>` is the address assigned to the associated Activity module
(e.g. `D1`, `M1`, `V1`...).  
These values can also be easily configured using the *HomeGenie Panel* app.

Options available for `CameraDisplayActivity` configuration:

| Key                  | Description                        | Default           |
|----------------------|------------------------------------|-------------------|
| `title-<address>`    | Displayed title/name               | `<address>`       |
| `rcam-<address>`     | Remote camera JPEG images feed URL |                   |
| `rcam-res-<address>` | Camera resolution (1 to 17)        | 3 remote, 5 local |
| `rcam-qlt-<address>` | Camera resolution (10 to 63)       | 10                |


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
