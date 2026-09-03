/*
* HomeGenie-Mini (c) 2018-2026 G-Labs
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

#ifndef HOMEGENIE_MINI_CC1101TRANSCEIVER_H
#define HOMEGENIE_MINI_CC1101TRANSCEIVER_H

#include <Config.h>
#include <cc1101.h>

#include "../configuration.h"

namespace IO { namespace RCS {

class CC1101Transceiver {
public:
    CC1101Transceiver() = default;
    ~CC1101Transceiver() {
        if (radio) {
            delete radio;
            radio = nullptr;
        }
    }

    /**
     * Reads settings and initializes the CC1101 module if present.
     * If detected, overrides rxPin and txPin with the GDO0 pin.
     *
     * @param rxPin Reference to the RX pin variable (overwritten if CC1101 is used).
     * @param txPin Reference to the TX pin variable (overwritten if CC1101 is used).
     * @return true if the CC1101 chip was detected and initialized, false otherwise.
     */
    bool begin(uint8_t& rxPin, uint8_t& txPin) {
        uint8_t ccModRxTxPin = Config::getSetting("ccmod-rxtx", CONFIG_CC1101_RxTxPin).toInt();
        if (ccModRxTxPin == 0) return false;

        uint8_t ccSpiCsPin = Config::getSetting("ccspi-cs", CONFIG_CC1101_SpiCsPin).toInt();
        uint8_t ccSpiMosiPin = Config::getSetting("ccspi-mosi", CONFIG_CC1101_SpiMosiPin).toInt();
        uint8_t ccSpiMisoPin = Config::getSetting("ccspi-miso", CONFIG_CC1101_SpiMisoPin).toInt();
        uint8_t ccSpiClkPin = Config::getSetting("ccspi-clk", CONFIG_CC1101_SpiClkPin).toInt();

        double ccRadioFrequency = Config::getSetting("cccfg-freq", CONFIG_CC1101_RadioFrequency).toDouble();
        if (ccRadioFrequency <= 0) ccRadioFrequency = 433.92;
        double ccRadioBandwidth = Config::getSetting("cccfg-bndw", CONFIG_CC1101_RadioBandwidth).toDouble();
        if (ccRadioBandwidth <= 0) ccRadioBandwidth = 270.0;
        int8_t ccRadioPower = Config::getSetting("cccfg-powr", CONFIG_CC1101_RadioPower).toInt();

#ifdef ESP32
        radio = new CC1101::Radio(ccSpiCsPin, ccSpiClkPin, ccSpiMisoPin, ccSpiMosiPin, ccModRxTxPin, PIN_UNUSED);
#else
        radio = new CC1101::Radio(ccSpiCsPin, ccModRxTxPin, PIN_UNUSED);
#endif

        if (radio->begin() != CC1101::STATUS_CHIP_NOT_FOUND) {
            radio->setModulation(CC1101::MOD_ASK_OOK);
            radio->setPacketFormat(CC1101::PKT_FORMAT_ASYNC_SERIAL);

            radio->setFrequency(ccRadioFrequency);
            radio->setOutputPower(ccRadioPower);
            radio->setRxBandwidth(ccRadioBandwidth);

            rxPin = txPin = ccModRxTxPin; // Single bidirectional pin (GDO0)

            Serial.println(F("CC1101 chip detected and initialized."));
            return true;
        }

        Serial.println(F("CC1101 chip not detected."));
        delete radio;
        radio = nullptr;
        return false;
    }

    /**
     * Starts listening in asynchronous RX mode.
     * Must be called after homeGenie->begin() to ensure the GPIO pin is left in INPUT mode.
     */
    void startListening() {
        if (radio) {
            radio->idle();
            radio->serialReceive();
        }
    }

    bool isDetected() const {
        return radio != nullptr;
    }

    bool isTransmitting() const {
        return transmitting;
    }

    /**
     * Automatically binds pre- and post-transmission callbacks to any transmitter instance (e.g. RCSwitch or X10).
     * Handles switching the radio between IDLE, TX, and RX states, and updates transmission status.
     *
     * @tparam T Transmitter type (must implement setBeforeTransmitCallback and setAfterTransmitCallback).
     * @param transmitter Pointer to the transmitter instance.
     */
    template <typename T>
    void bindTransmitter(T* transmitter) {
        if (radio && transmitter) {
            transmitter->setBeforeTransmitCallback([this]() {
                this->transmitting = true;
                this->radio->idle();
                this->radio->serialTransmit();
            });
            transmitter->setAfterTransmitCallback([this]() {
                this->radio->idle();
                this->radio->serialReceive();
                this->transmitting = false;
            });
        }
    }

private:
    CC1101::Radio* radio{nullptr};
    volatile bool transmitting{false};
};

}} // namespace IO::RCS

#endif // HOMEGENIE_MINI_CC1101TRANSCEIVER_H
