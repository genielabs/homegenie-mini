// Default config

#ifdef ESP32_S3
#define CONFIG_X10RFReceiverPin 7
#define CONFIG_X10RFTransmitterPin 5
#else
#define CONFIG_X10RFReceiverPin 6
#define CONFIG_X10RFTransmitterPin 4
#endif
#define CONFIG_X10RF_MODULE_ADDRESS        "RF"
