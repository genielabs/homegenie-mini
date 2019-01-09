#ifndef HOMEGENIE_MINI_X10_RECEIVER_H_
#define HOMEGENIE_MINI_X10_RECEIVER_H_

#include "Arduino.h"
#include "ReceiverConfig.h"

namespace X10 {

#define X10_RF_REPEAT_THRESHOLD   30
#define X10_RF_REPEAT_MAX   1000


    class Receiver {
    public:
        class X10RfDataReceivedCallback {
        public:
            virtual int onX10RfDataReceived(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
        };

        Receiver();

        Receiver(ReceiverConfig *configuration, X10RfDataReceivedCallback *rfReceiveCallback);

        void begin();

        void receive();

        void enable();

        void disable();

        bool isEnabled();

    private:
        bool enabled;
        ReceiverConfig *configuration;
        X10RfDataReceivedCallback *rfReceiveCallback;
        // 32-bit RF message decoding
        uint32_t riseUs;
        int8_t receivedCount;
        uint32_t receiveBuffer;

        // TODO: move to an utility class (maybe static)
        // Utility methods
        uint32_t reverseBits(uint32_t n);
        uint8_t reverseByte(uint8_t n);
    };

}

#endif // HOMEGENIE_MINI_X10_RECEIVER_H_
