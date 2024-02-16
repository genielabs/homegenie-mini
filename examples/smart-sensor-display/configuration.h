#ifdef ESP8266

    #define CONFIG_DHTxx_DataPin 0
    #define CONFIG_LightSensorPin 17

#elif MINI_ESP32

    #define CONFIG_DHTxx_DataPin 17
    #define CONFIG_LightSensorPin 36

#else

    #define CONFIG_DHTxx_DataPin 21
    #define CONFIG_LightSensorPin 34

#endif
