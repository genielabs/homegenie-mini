# HomeGenie Mini

HomeGenie mini is an open source hardware + firmware solution for building smart devices
based on the popular *ESP8266 chip*, a WiFi capable micro controller.
This project can can be employed for a variety of Home Automation and IoT applications.

**Hardware features**

- Quick install via WPS button and automatic discovery (SSDP)
- Temperature and light sensing
- RF transceiver (315/330/433Mhz) currently implementing **X10** home automation protocol
- P1 connector with 4 GPIO configurable as SPI/DIO/PWM

**Software features**

- HTTP API (subset of standard HomeGenie API)
- Real time event stream over WebSocket or SSE connection
- MQTT broker over websocket
- SSDP
- NTP client for time sync
- Scripting engine (currently work in progress)

**Mobile client**

- HomeGenie plus for Android

The P1 connector can be used for various applications such as:

- control a led brightness or drive a motor at different speed (PWM)
- control up to 4 relays to actuate lights and appliances (DIO)
- hosting other sensors or interfacing with other hardware (SPI)
- breadboard playground

## Building HomeGene Mini

![HomeGenie mini PCB front and rear](https://raw.githubusercontent.com/genielabs/homegenie-mini/master/pcb/homegenie_mini_v1_1_pcb.png)

### Hardware

Gerber files required for printing the circuit board are located in the `./pcb` folder.

- https://github.com/genielabs/homegenie-mini/tree/master/pcb

The PCB size is 44x50mm.

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

The firmware source code is built using *Platform.IO* and based on *Espressif*
platform and *Arduino* framework.

**Source code**

- https://github.com/genielabs/homegenie-mini/tree/master/src

**Release**

- https://github.com/genielabs/homegenie-mini/releases/


## Disclaimer

<small><code>
THIS PROJECT IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS PROJECT, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
</code></small>
