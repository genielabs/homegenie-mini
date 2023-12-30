#ifdef ESP8266

    #define CONFIG_DS18B20_DataPin 0
    #define CONFIG_LightSensorPin 17

#elif MINI_ESP32

    #define CONFIG_DS18B20_DataPin 17
    #define CONFIG_LightSensorPin 36

#else

    #define CONFIG_DS18B20_DataPin 0
    #define CONFIG_LightSensorPin 34

#endif
