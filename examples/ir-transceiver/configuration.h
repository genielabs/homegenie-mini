#define CONFIG_IRReceiverPin 5
#define CONFIG_IRTransmitterPin 7

#ifndef CONFIG_StatusLedNeoPixelPin
#define CONFIG_StatusLedNeoPixelPin 10
#endif

#define CONFIG_IR_MODULE_ADDRESS   "IR"

#ifdef MINI_ESP32

#define CONFIG_IRTransmitterPin 22
#define CONFIG_IRReceiverPin 21

#endif
