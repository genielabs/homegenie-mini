#define CONFIG_IRReceiverPin 5
#define CONFIG_IRTransmitterPin 7

#ifndef CONFIG_StatusLedNeoPixelPin
#ifdef ESP32_C3
#define CONFIG_StatusLedNeoPixelPin 10
#endif
#endif

#define CONFIG_IR_MODULE_ADDRESS   "IR"

#ifdef MINI_ESP32

#define CONFIG_IRTransmitterPin 22
#define CONFIG_IRReceiverPin 21

#endif
