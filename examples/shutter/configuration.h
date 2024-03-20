#ifndef CONFIG_StatusLedNeoPixelPin
#define CONFIG_StatusLedNeoPixelPin 10
#endif

// TODO: #define CONFIG_StepperMotorPinXX YY

#ifdef ESP32_C3
    #define CONFIG_ServoMotorPin 5
#else
    #define CONFIG_ServoMotorPin 15
#endif
