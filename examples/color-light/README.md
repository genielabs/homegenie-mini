# color-light

A smart light device with addressable RGB LEDs.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-led-lights/)


## Firmware configuration (in addition to default system options)

| Key        | Description                | Default                                 |
|------------|----------------------------|-----------------------------------------|
| `leds-pin` | LEDs strip GPIO pin number | -1 (-1=not used)                        |
| `leds-cnt` | Number of LEDs             | 1 (1 - 200)                             |
| `stld-typ` | LEDs type                  | RGB/RGBW order mask (see code for ref.) |
| `leds-spd` | Data transfer speed        | 0 (0=800kHz, 256=400kHz)                |
| `stld-pin` | Status LED (RGB) pin       | -1 (-1=not used)                        |
| `stld-typ` | Status LED type            | RGB/RGBW order mask (see code for ref.) |
| `stld-spd` | Status LED speed           | 0 (0=800kHz, 256=400kHz)                |

*Example **setting** configuration from a terminal connected to the serial port of the device:*

```
#SET:leds-pin=5
#SET:leds-cnt=25
#SET:leds-typ=6
#SET:leds-spd=0
#SET:stld-pin=10
#SET:stld-typ=82
#SET:stld-spd=0
#RESET
```

*Example **getting** configuration value:*

```
#GET:stl-pin
```
response:
```
#GET:stl-pin=10
```

## Manual build and install

You can manually build and install the firmware from source code
as explained in the [Getting started](../../getting-started#custom-firmware) page
and using the following command for flashing the firmware:

```bash
pio run -e color-light[<target>] -t upload
```

where the optional `<target>` suffix can be one of the following: `-c3`, `-d1-mini`.