# Build platform policy

The default ESP32 platform is `espressif32@~8.0.0` (Arduino 3.x).
All ESP32-C3 environments explicitly use the shared `esp32_c3` profile pinned to
`espressif32@7.1.3` (Arduino 2.x). ESP8266 environments remain on their separate
`espressif8266@2.6.3` platform. S2, S3, C6 and other ESP32 targets keep the default.

## ESP32-C3 environments

- `default-c3`
- `smart-sensor-c3`
- `rf-transceiver-c3`
- `ir-transceiver-c3`
- `ir-rf-transceiver-c3`
- `color-light-c3`
- `smart-motor-c3`
- `playground-c3`

The profile selects platform and compiler flags together so SystemInfo reports
7.1.3 for C3 rather than the global 8.x default. Common compiler flags are shared
with the other profiles. GPIO defaults, peripherals, automation, Wi-Fi and
Bluetooth provisioning remain enabled as before; the workaround changes only
the C3 build platform, not the firmware's application behavior.

## Why C3 remains on Arduino 2.x

On the maintainer's ESP32-C3 with a 256-LED matrix and Wi-Fi connected, current
sources built with 7.1.3 reported 126,120 bytes free internal DRAM. An 8.0.0 build
with Bluetooth completely excluded still reported 99,552 bytes, a 26,568-byte
shortfall despite omitting a feature. Earlier 8.0.0 observations were around
78 KB free, but included an offline measurement and must not be treated as an
exact connected-versus-connected BLE comparison.

The same-source ELF comparison found:

| Build | Reserved IRAM | Data + BSS | Combined static SRAM |
| --- | ---: | ---: | ---: |
| 7.1.3, Bluetooth included | 71,168 B | 71,612 B | 142,780 B |
| 8.0.0, Bluetooth included | 91,136 B | 51,272 B | 142,408 B |
| 8.0.0, Bluetooth excluded | 66,560 B | 49,536 B | 116,096 B |

Static totals do not identify the entire runtime heap difference: SDK runtime
allocations and the regions returned by Bluetooth deinitialization also matter.
The current pin preserves headroom for Duktape while that investigation remains
open. This is a target-specific memory decision, not a rollback of the Arduino
3.x compatibility fixes or the PSRAM improvements on supported boards.

## Diagnostic build

`platformio.memory-nobt.ini`, when present, is an optional diagnostic profile
explicitly pinned to 8.0.0. It excludes Bluetooth even in BOOT 2 and is not used
for normal release builds. Production C3 builds continue to include Bluetooth.
The diagnostic report and firmware were saved locally under
`artifacts/memory-comparison/`; they are not required to build the project.

## Future re-evaluation

Reconsider the C3 pin after measuring an updated core on the same board,
configuration and workload, with enough free internal heap and contiguous
allocation space for JavaScript. Verify the full build matrix and device
BOOT/WPS/Bluetooth paths before the next release. ESP8266 is not affected by
the espressif32 migration and does not use espressif32@7.1.3.
