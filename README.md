[![Travis Build Status](https://travis-ci.org/genielabs/homegenie-mini.svg?branch=master)](https://travis-ci.org/genielabs/homegenie-mini)

# HomeGenie Mini

HomeGenie mini *(code name **Sbirulino**)* is an open hardware + firmware solution for building smart devices
based on the popular *ESP8266 chip*, a WiFi capable micro controller.

**Hardware features**

- WPS button for quick WiFi setup
- Temperature and light sensors
- RF transceiver (315/330/433Mhz)
- Expansion connector (P1) with 4 GPIO configurable as SPI/DIO/PWM

*Example applications of the P1 connector:*

- control the brightness of a led or drive a motor at different speed (PWM)
- control up to 4 relays to actuate lights and appliances (DIO)
- hosting additional sensors, connecting a display or other hardware (SPI)
- breadboard playground

**Firmware features**

- Automatic discovery (SSDP) for instant client setup
- X10 home automation RF protocol encoding and decoding with mapping to virtual modules
- GPIO mapping to virtual modules: switch for digital output, dimmer for analog output or sensor for inputs (work in progress)
- Modules state persistence
- HTTP API (subset of standard HomeGenie API)
- Real time event stream over WebSocket or SSE connection
- MQTT broker over websocket
- NTP client for time sync
- Scripting engine (work in progress)
- Serial CLI

**Mobile client**

- [HomeGenie plus](https://play.google.com/store/apps/details?id=com.glabs.homegenieplus&hl=en_US) for Android


### HomeGenie API

HomeGenie Mini API is a subset of HomeGenie Server API that makes HomeGenie Mini a real
fully working light version of HomeGenie Server specifically designed for micro controllers.

#### [HomeAutomation.HomeGenie](https://genielabs.github.io/HomeGenie/api/mig/core_api_config.html)

Implemented subset:

- [`/api/HomeAutomation.HomeGenie/Logging/RealTime.EventStream/`](https://genielabs.github.io/HomeGenie/api/mig/core_api_logging.html#1)
- [`/api/HomeAutomation.HomeGenie/Config/Modules.Get`](https://genielabs.github.io/HomeGenie/api/mig/core_api_config.html#2)
- [`/api/HomeAutomation.HomeGenie/Config/Modules.List`](https://genielabs.github.io/HomeGenie/api/mig/core_api_config.html#3)
- [`/api/HomeAutomation.HomeGenie/Config/Groups.List`](https://genielabs.github.io/HomeGenie/api/mig/core_api_config.html#4)

`EXAMPLE Request`
```
GET /api/HomeAutomation.HomeGenie/Config/Modules.Get/HomeAutomation.HomeGenie/mini
```

`Response`
```
{
  "Name": "HG-Mini",
  "Description": "HomeGenie Mini node",
  "DeviceType": "Sensor",
  "Domain": "HomeAutomation.HomeGenie",
  "Address": "mini",
  "Properties": [{
    "Name": "Sensor.Luminance",
    "Value": "114",
    "Description": "",
    "FieldType": "",
    "UpdateTime": "2019-01-30T13:34:02.293Z"
  },{
    "Name": "Sensor.Temperature",
    "Value": "18.25",
    "Description": "",
    "FieldType": "",
    "UpdateTime": "2019-01-30T13:34:02.293Z"
  }],
  "RoutingNode": ""
}
```

#### [HomeAutomation.X10](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html)

Implemented subset:

- [`/api/HomeAutomation.X10/<module_address>/Control.On`](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html#1)
- [`/api/HomeAutomation.X10/<module_address>/Control.Off`](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html#2)
- [`/api/HomeAutomation.X10/<module_address>/Control.Level`](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html#5)
- [`/api/HomeAutomation.X10/<module_address>/Control.Toggle`](https://genielabs.github.io/HomeGenie/api/mig/mig_api_x10.html#6)


#### HomeGenie Mini specific API

It's possible to control the 4 GPIOs on the `P1` expansion port using the following API:

- `/api/HomeAutomation.HomeGenie/<pin_name>/Control.On`
- `/api/HomeAutomation.HomeGenie/<pin_name>/Control.Off`
- `/api/HomeAutomation.HomeGenie/<pin_name>/Control.Level/<level>`

Where `<pin_name>` can be `D5`, `D6`, `D7` or `D8` and `<level>` a integer between `0` and `100`.

**Examples**

```
# Set output D6 to 50% (1.65V)
/api/HomeAutomation.HomeGenie/D6/Control.Level/50

# Set output D5 to 100% (3.3V)
/api/HomeAutomation.HomeGenie/D5/Control.Level/100
# or
/api/HomeAutomation.HomeGenie/D5/Control.On

# Set output D8 to 0% (0V)
/api/HomeAutomation.HomeGenie/D8/Control.Level/0
# or
/api/HomeAutomation.HomeGenie/D8/Control.Off
```


## Building HomeGene Mini

![HomeGenie mini PCB front and rear](https://raw.githubusercontent.com/genielabs/homegenie-mini/master/pcb/homegenie_mini_v1_1_pcb.png)

### Hardware

Gerber files required for printing the circuit board are located in the `./pcb` folder.

- https://github.com/genielabs/homegenie-mini/tree/master/pcb

The PCB size is 44mm*50mm.

**Components listing**

- 1 ESP8266 WeMoo D1 mini WiFi module (or equivalent)
- 1 DS18B20 (temperature sensor)
- 1 LDR (light sensor)
- 1 FS1000A (RF transmitter)
- 1 XY-MK-5V (RF receiver)
- 1 R1 10kΩ
- 1 R2 4.7kΩ
- 1 R3 330Ω
- 1 R4 330Ω
- 2 LED 3mm
- 1 Push button 6mm*6mm

### Firmware

The firmware source code is built using [*Platform.IO IDE*](https://docs.platformio.org/en/latest/ide/pioide.html) and based on **Espressif**
platform and **Arduino** framework.

**Source code**

- https://github.com/genielabs/homegenie-mini/tree/master/src

**Release**

- https://github.com/genielabs/homegenie-mini/releases/


## Disclaimer

<small><code>THIS PROJECT IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS PROJECT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</code></small>
