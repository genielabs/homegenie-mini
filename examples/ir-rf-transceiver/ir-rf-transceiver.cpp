/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
 *
 *
 * This file is part of HomeGenie-Mini (HGM).
 *
 *  HomeGenie-Mini is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HomeGenie-Mini is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HomeGenie-Mini.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Authors:
 * - Generoso Martello <gene@homegenie.it>
 *
 *
 */

#include <HomeGenie.h>

#include "configuration.h"

#include "../ir-transceiver/io/IRReceiver.h"
#include "../ir-transceiver/io/IRTransmitter.h"
#include "../ir-transceiver/api/IRTransceiverHandler.h"

#include "../rf-transceiver/io/RFReceiver.h"
#include "../rf-transceiver/io/RFTransmitter.h"
#include "../rf-transceiver/api/RCSwitchHandler.h"

#include "../x10-transceiver/io/X10RFReceiver.h"
#include "../x10-transceiver/io/X10RFTransmitter.h"
#include "../x10-transceiver/api/X10Handler.h"

#ifdef BOARD_HAS_RGB_LED
#include "../color-light/StatusLed.h"
StatusLed statusLed;
#endif

using namespace Service;

HomeGenie* homeGenie;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Firefly IR-RF";

    homeGenie = HomeGenie::getInstance();
    auto miniModule = homeGenie->getDefaultModule();
    miniModule->setProperty(Implements::Scheduling, "true");

#ifdef BOARD_HAS_RGB_LED
    // Custom status led (builtin NeoPixel RGB LED)
    statusLed.setup();
#endif

    // IR transceiver

    auto irApiHandler = new IRTransceiverHandler();
    // IR receiver
    uint8_t irReceiverPin = Config::getSetting("irrc-pin", CONFIG_IRReceiverPin).toInt();
    if (irReceiverPin > 0) {
        auto receiverConfiguration = new IR::IRReceiverConfig(irReceiverPin);
        auto receiver = new IR::IRReceiver(receiverConfiguration);
        irApiHandler->setReceiver(receiver);
        homeGenie->addIOHandler(receiver);
    }
    // IR transmitter
    uint8_t irTransmitterPin = Config::getSetting("irtr-pin", CONFIG_IRTransmitterPin).toInt();
    if (irTransmitterPin > 0) {
        auto transmitterConfig = new IR::IRTransmitterConfig(irTransmitterPin);
        auto transmitter = new IR::IRTransmitter(transmitterConfig);
        irApiHandler->setTransmitter(transmitter);
    }
    homeGenie->addAPIHandler(irApiHandler);

    uint8_t rfReceiverPin = Config::getSetting("rfrc-pin", CONFIG_RCSwitchReceiverPin).toInt();
    uint8_t rfTransmitterPin = Config::getSetting("rftr-pin", CONFIG_RCSwitchTransmitterPin).toInt();

    // RC-Switch RF transceiver

    auto rcsApiHandler = new RCSwitchHandler();
#ifdef BOARD_HAS_RGB_LED
    rcsApiHandler->setOnDataReady([](const char* c) {
        statusLed.signalActivity(0, 255, 255);
    });
#endif

    // RCSwitch RF Transmitter
    if (rfTransmitterPin > 0) {
        auto rcsTransmitterConfig = new RCS::RFTransmitterConfig(rfTransmitterPin);
        auto rcsTransmitter = new RCS::RFTransmitter(rcsTransmitterConfig);
        rcsApiHandler->setTransmitter(rcsTransmitter);
    }
    // RCSwitch RF Receiver
    if (rfReceiverPin > 0) {
        auto rcsReceiverConfiguration = new RCS::RFReceiverConfig(rfReceiverPin);
        auto rcsReceiver = new RCS::RFReceiver(rcsReceiverConfiguration);
        homeGenie->addIOHandler(rcsReceiver);
        // shared interrupt handler with X10 class
        X10RFReceiver::addInterruptHandler([rcsReceiver](){
            rcsReceiver->handleInterrupt();
        });
        rcsApiHandler->setReceiver(rcsReceiver);
    }

    homeGenie->addAPIHandler(rcsApiHandler);

    // X10 RF transceiver

    auto x10ApiHandler = new X10Handler();
#ifdef BOARD_HAS_RGB_LED
    x10ApiHandler->setOnDataReady([](const char* c) {
        statusLed.signalActivity(0, 0, 255);
    });
#endif

    // X10 RF RFTransmitter
    if (rfTransmitterPin > 0) {
        auto x10TransmitterConfig = new X10::X10RFTransmitterConfig(rfTransmitterPin);
        auto x10Transmitter = new X10::X10RFTransmitter(x10TransmitterConfig);
        x10ApiHandler->setTransmitter(x10Transmitter);
    }
    // X10 RF RFReceiver
    if (rfReceiverPin > 0) {
        auto x10ReceiverConfig = new X10::X10RFReceiverConfig(rfReceiverPin);
        auto x10Receiver = new X10::X10RFReceiver(x10ReceiverConfig);
        x10ApiHandler->setReceiver(x10Receiver);
        homeGenie->addIOHandler(x10Receiver);
    }

    homeGenie->addAPIHandler(x10ApiHandler);

    homeGenie->begin();
}


void loop()
{
    homeGenie->loop();
}
