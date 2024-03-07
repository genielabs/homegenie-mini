/*
 * HomeGenie-Mini (c) 2018-2024 G-Labs
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
 */

/*
  Simple example for receiving

  https://github.com/sui77/rc-switch/
*/

#include "IRTransmitter.h"
#include "Utility.h"

namespace IO { namespace IR {

    IRTransmitter::IRTransmitter() {
        setLoopInterval(100);
    }

    IRTransmitter::IRTransmitter(IRTransmitterConfig *configuration) : IRTransmitter() {
        this->configuration = configuration;
    }

    void IRTransmitter::begin() {
        Logger::info("|  - IO::IR::IRTransmitter (PIN=%d)", configuration->getPin());
        irSend = new IRsend(configuration->getPin());
        irSend->begin();
        Logger::info("|  ✔ IO::IR::IRTransmitter");
    }

    void IRTransmitter::loop() {
        if (sendCount > 0 && millis() - lastSentTs > sendDelay) {
            // TODO: ... log
            if (sendProtocol == UNKNOWN) {
                irSend->sendRaw((uint16_t*)rawData, dataSize, 38000);
            } else if(hasACState(sendProtocol)) {
                irSend->send(sendProtocol, (uint8_t*)rawData, dataSize);
            } else {
                irSend->send(sendProtocol, (*(uint64_t*)rawData), dataSize);
            }
            sendCount--;
            lastSentTs = millis();
            setLoopInterval(1);
        }
        if (sendCount == 0 && rawData != nullptr) {
            _free();
            if (listener != nullptr) listener->onSendComplete();
            setLoopInterval(100);
        }
    }

    void IRTransmitter::sendCommand(String& command, uint8_t repeat, uint16_t delay) {

        if (rawData != nullptr) _free();

        auto protocol = (decode_type_t)strtol(command.substring(0, 4).c_str(), nullptr, 16);
        auto size = strtol(command.substring(4, 8).c_str(), nullptr, 16);
        command = command.substring(8);

        sendProtocol = protocol;
        if (sendProtocol == UNKNOWN) {
            size_t dataMem = (size_t)size * sizeof(uint16_t);
            rawData = (uint16_t*)malloc(dataMem);
            Utility::getBytes(command, (uint16_t*)rawData);
        } else if (hasACState(sendProtocol)) {
            size_t dataMem = (size_t)size * sizeof(uint8_t);
            rawData = (uint8_t*)malloc(dataMem);
            Utility::getBytes(command, (uint8_t*)rawData);
        } else {
            auto* l = new uint64_t(strtol(command.c_str(), nullptr, 16));
            rawData = l;
        }

        dataSize = size;
        sendCount = repeat;
        sendDelay = delay;

        if (listener != nullptr) listener->onSendStart();

    }

    void IRTransmitter::setListener(IRTransmitterListener *l) {
        listener = l;
    }

    void IRTransmitter::_free() {
        if (sendProtocol == UNKNOWN || hasACState(sendProtocol)) {
            free(rawData);
        } else {
            delete (uint64_t*)rawData;
        }
        rawData = nullptr;
    }

}}
