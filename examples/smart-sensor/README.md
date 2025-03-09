# smart-sensor

A smart multi-sensor device.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-sensor/)


## Firmware configuration

In addition to default system options the following configuration options are available:

| Key         | Description                          | Default                                 |
|-------------|--------------------------------------|-----------------------------------------|
| `stld-pin`  | Status LED (RGB) pin                 | -1 (-1=not used)                        |
| `stld-typ`  | Status LED type                      | RGB/RGBW order mask (see code for ref.) |
| `stld-spd`  | Status LED speed                     | 0 (0=800kHz, 256=400kHz)                |
| `ligh-typ`  | Light sensor type                    | -1                                      |
| `ligh-pin`  | Light sensor GPIO#                   | -1                                      |
| `motn-typ`  | Motion sensor type                   | -1                                      |
| `motn-pin`  | Motion sensor GPIO#                  | -1                                      |
| `soth-typ`  | Temperature sensor type              | -1                                      |
| `soth-pin`  | Temperature sensor GPIO#             | -1                                      |
| `sdht-typ`  | Temperature + Humidity sensor type   | -1                                      |
| `sdht-pin`  | Temperature + Humidity sensor GPIO#  | -1                                      |
| `sdht-adj`  | Adjust temp. read value (-10 to +10) | 0                                       |
| `colr-typ`  | Color sensor type                    | -1                                      |
| `colr-sda`  | Color sensor SDA pin                 | -1                                      |
| `colr-scl`  | Color sensor SCL pin                 | -1                                      |
| `camr-typ`  | Camera sensor type (ESP32)           | "" ("esp32-cam" to enable camera)       |   
| `camr-pwr`  | Camera Power down signal GPIO#       | -1                                      |  
| `camr-xcl`  | Camera XClock GPIO#                  | 15                                      | 
| `camr-xfr`  | Camera XClock frequency Hz           | 20000000 (16000000 - 24000000)          | 
| `camr-vsn`  | Camera Vertical sync. GPIO#          | 6                                       | 
| `camr-hrf`  | Camera Horizontal sync. ref. GPIO#   | 7                                       |
| `camr-pcl`  | Camera Pixel clock GPIO#             | 13                                      |
| `camr-sda`  | Camera SDA GPIO#                     | 4                                       |
| `camr-scl`  | Camera SCL GPIO#                     | 5                                       |
| `camr-cd0`  | Camera D0  GPIO#                     | 11                                      | 
| `camr-cd1`  | Camera D1  GPIO#                     | 9                                       |
| `camr-cd2`  | Camera D2  GPIO#                     | 8                                       |
| `camr-cd3`  | Camera D3  GPIO#                     | 10                                      |
| `camr-cd4`  | Camera D4  GPIO#                     | 12                                      |
| `camr-cd5`  | Camera D5  GPIO#                     | 18                                      |
| `camr-cd6`  | Camera D6  GPIO#                     | 17                                      |
| `camr-cd7`  | Camera D7  GPIO#                     | 16                                      |
| `io-typ01`  | Output #1 type                       | "Dimmer" ("Dimmer", "Switch", "Light")  |                           
| `io-pin01`  | Output #1 GPIO#                      | -1                                      |
| `io-typ02`  | Output #2 type                       | "Dimmer" ("Dimmer", "Switch", "Light")  |
| `io-pin02`  | Output #2 GPIO#                      | -1                                      | 

(*) default camera settings are valid for *"ESP32-S3-CAM"* kit model 


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
- ESP32 (generic + PSRAM)  
  `-psram`
- ESP32-C3  
  `-c3`
- ESP32-S3  
  `-s3`
- ESP32-S3 N16R8V  
  `-s3-n16r8v`
- ESP32 D1 Mini    
  `-d1-mini-esp32`
