// Default config

#ifdef ESP32_S3
#define CONFIG_IRReceiverPin "8"
#define CONFIG_IRTransmitterPin "6"
#else
#define CONFIG_IRReceiverPin "7"
#define CONFIG_IRTransmitterPin "5"
#endif
#define CONFIG_IR_MODULE_ADDRESS   "IR"
