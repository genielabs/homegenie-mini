//
// Created by gene on 16/02/19.
//

#ifndef HOMEGENIE_MINI_CONFIG_H
#define HOMEGENIE_MINI_CONFIG_H

#include <Arduino.h>

class Config {
public:
    const static uint8_t ServiceButtonPin = D4;
    const static uint8_t StatusLedPin = D0;
    const static uint16_t WpsModePushInterval = 2500;
    const static bool X10RFReceiverEnabled = false;
};

#endif //HOMEGENIE_MINI_CONFIG_H
