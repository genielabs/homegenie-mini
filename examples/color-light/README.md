# color-light

A smart light device with addressable RGB LEDs.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-led-lights/)


## Firmware configuration

In addition to default system options the following configuration options are available:

| Key        | Description                 | Default                                 |
|------------|-----------------------------|-----------------------------------------|
| `leds-pin` | LED strip GPIO pin number   | -1 (-1=not used)                        |
| `leds-cnt` | Number of LEDs              | 1 (1 - 200)                             |
| `stld-typ` | LEDs type                   | RGB/RGBW order mask (see code for ref.) |
| `leds-spd` | Data transfer speed         | 0 (0=800kHz, 256=400kHz)                |
| `stld-pin` | Status LED (RGB) pin        | -1 (-1=not used)                        |
| `stld-typ` | Status LED type             | RGB/RGBW order mask (see code for ref.) |
| `stld-spd` | Status LED speed            | 0 (0=800kHz, 256=400kHz)                |
| `btn1-pin` | Toggle button GPIO pin num. | -1 (-1=not used)                        |
| `btn2-pin` | Mode button GPIO pin num.   | -1 (-1=not used)                        |


## Manual build and install

You can manually build and install the firmware from source code
as explained in the [Getting started](../../getting-started#custom-firmware) page
and using the following command for flashing the firmware:

```bash
pio run -e color-light[<target>] -t upload
```

where the optional `<target>` suffix can be one of the following: `-c3`, `-d1-mini`.