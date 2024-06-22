
# smart-sensor

Smart multi-sensor device with display. HomeGenie Mini UI example use.

- [Documentation and firmware install page](https://homegenie.it/mini/1.2/examples/smart-sensor/)


## Firmware configuration (in addition to default system options)

| Key        | Description                         | Default |
|------------|-------------------------------------|---------|
| `ligh-typ` | Light sensor type                   | -1      |
| `ligh-pin` | Light sensor GPIO#                  | -1      |
| `motn-typ` | Motion sensor type                  | -1      |
| `motn-pin` | Motion sensor GPIO#                 | -1      |
| `soth-typ` | Temperature sensor type             | -1      |
| `soth-pin` | Temperature sensor GPIO#            | -1      |
| `sdht-typ` | Temperature + Humidity sensor type  | -1      |
| `sdht-pin` | Temperature + Humidity sensor GPIO# | -1      |


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
- ESP32-S3 + display
  `-display-s3`