// Default config

#ifdef ESP32_S3
#define CONFIG_RCSwitchReceiverPin 7
#define CONFIG_RCSwitchTransmitterPin 5
#else
#define CONFIG_RCSwitchReceiverPin 6
#define CONFIG_RCSwitchTransmitterPin 4
#endif
#define CONFIG_RCSwitchRF_MODULE_ADDRESS   "RF"
