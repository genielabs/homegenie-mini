// Default config

#define CONFIG_CC1101_RadioFrequency "433.92"
#define CONFIG_CC1101_RadioBandwidth "270.0"
#define CONFIG_CC1101_RadioPower "10"

#ifdef ESP32_S3
#define CONFIG_RCSwitchReceiverPin "7"
#define CONFIG_RCSwitchTransmitterPin "5"
#define CONFIG_CC1101_RxTxPin "4" // 0 = CC1101 disabled
#define CONFIG_CC1101_SpiCsPin "9"
#define CONFIG_CC1101_SpiMosiPin "10"
#define CONFIG_CC1101_SpiMisoPin "2"
#define CONFIG_CC1101_SpiClkPin "3"
#else
#define CONFIG_RCSwitchReceiverPin "6"
#define CONFIG_RCSwitchTransmitterPin "4"
#define CONFIG_CC1101_RxTxPin "3" // 0 = CC1101 disabled
#define CONFIG_CC1101_SpiCsPin "8"
#define CONFIG_CC1101_SpiMosiPin "9"
#define CONFIG_CC1101_SpiMisoPin "1"
#define CONFIG_CC1101_SpiClkPin "2"
#endif
#define CONFIG_RCSwitchRF_MODULE_ADDRESS   "RF"

#include "../x10-transceiver/configuration.h"
