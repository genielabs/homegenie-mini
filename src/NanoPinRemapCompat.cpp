#include <Arduino.h>

#if defined(ARDUINO_NANO_ESP32) && defined(BOARD_HAS_PIN_REMAP) && \
    !defined(BOARD_USES_HW_GPIO_NUMBERS)
#if defined(ESP_ARDUINO_VERSION) && defined(ESP_ARDUINO_VERSION_VAL)
#if ESP_ARDUINO_VERSION == ESP_ARDUINO_VERSION_VAL(3, 3, 12)

// Arduino ESP32 3.3.12 exposes pin remapping with C++ linkage, while
// C libraries such as NeoPixel's esp.c need the unmangled C symbol.
// Keep the core's existing Arduino-pin-to-GPIO mapping.
extern "C" int8_t nanoDigitalPinToGPIO(int8_t pin)
    __asm__("digitalPinToGPIONumber");

extern "C" int8_t nanoDigitalPinToGPIO(int8_t pin) {
    return digitalPinToGPIONumber(pin);
}

#endif
#endif
#endif
