# smart-motor

A smart motor device and generic servo motor driver. Supports accurate positioning
also using continuous rotation servos by using a GPIO switch as encoder pulse.
The default version of this firmware can control up to 8 motors simultaneously.


- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/fpv-rc-car/)


## Firmware configuration

In addition to default system options the following configuration options are available,
where `<n>` is the number of the motor (from 1 to 8):

| Key            | Description                                    | Default                                 |
|----------------|------------------------------------------------|-----------------------------------------|
| `stld-pin`     | Status LED (RGB) pin                           | -1 (-1=not used)                        |
| `stld-typ`     | Status LED type                                | RGB/RGBW order mask (see code for ref.) |
| `stld-spd`     | Status LED speed                               | 0 (0=800kHz, 256=400kHz)                |
| `motr-typ#<n>` | Motor type (Servo or ServoEncoder)             | Servo                                   |   
| `motr-pin#<n>` | Motor signal GPIO#                             | 6                                       |
| `rcrcar-4wd`   | Enable configuration for FPV-RC-Car project    |                                         | 
| `robotc-arm`   | Enable configuration for Robot-Arm project     |                                         |


When `rcrcar-4wd` is set to `true` motors `S1`, `S2`, `S3`, `S4` will be configured to
operate at maximum speed (10). It will also add `Motion Control` and `Steering Control`
modules for precise control of the *FPV-RC-Car* motion and steering.

When `robotc-arm` is set to `true` and also `rcrcar-4wd` is set to `true`, then
motors `S5`, `S6`, `S7`, `S8` will be configured to operate at half speed (5) to
enable smoother *Robotic Arm* movements. Otherwise, if `rcrcar-4wd` is set to `false`
then the configured motors will be `S1`, `S2`, `S3`, `S4`.


### Parameters for `motr-typ`= '`ServoEncoder`'

| Key            | Description               | Default                          |
|----------------|---------------------------|----------------------------------|
| `motr-spd#<n>` | Motor speed (1-10)        | 5                                |
| `encd-pin#<n>` | Encoder switch GPIO#      | -1 (emulated encoder step pulse) |
| `encd-stp#<n>` | Maximum number of steps   | 15                               |


### Parameters for `motr-typ`= '`Servo`'

| Key            | Description          | Default                                 |
|----------------|----------------------|-----------------------------------------|
| `angl-spd#<n>` | Seeking speed (1-10) | 5                                       |
| `angl-min#<n>` | Start angle (0-179)  | 0                                       |
| `angl-max#<n>` | End angle (1-180)    | 180                                     |


### Manual build and install

You can also manually build and install the firmware from source code
as explained in the [Getting started](https://homegenie.it/mini/1.2/getting-started#custom-firmware) page
and using the following commands for flashing the firmware:

```bash
pio run -e smart-motor[<target>] -t upload
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
- Arduino Nano ESP32-S3
  `-s3-nano`
- ESP32 D1 Mini    
  `-d1-mini-esp32`
