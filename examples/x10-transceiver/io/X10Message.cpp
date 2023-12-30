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
 *
 * Based on Michael LeMay's x10-cm19a
 * - https://github.com/amccool/x10-cm19a/blob/master/drivers/usb/comm/x10-cm19a.c
 *
 *
 * Releases:
 * - 2019-01-10 Initial release
 *
 */

#include "X10Message.h"

/* Maximum debugging level to actually display */
#define DBG_MAX DBG_INFO
#define MSG_PFX "x10"
#define DEBUG 1

#if DEBUG
#define debug(sev, format, args...)                    \
    do { printk(MSG_PFX format, ## args); } while (0)
#else
#define debug(sev, format, args...)					\
    do { if (sev <= DBG_MAX) printk(KERN_DEBUG MSG_PFX format, ## args); } while (0)
#endif
#define error(format, args...) printk(KERN_ERR MSG_PFX format, ## args)
#ifdef DEBUG
#define dbg(format, arg...)                                             \
        printk(KERN_DEBUG "%s: " format "\n", __FILE__, ##arg)
#else
#define dbg(format, arg...)                                             \
do {                                                                    \
        if (0)                                                          \
                printk(KERN_DEBUG "%s: " format "\n", __FILE__, ##arg); \
} while (0)
#endif

namespace IO {
    namespace X10 {

            int X10Message::encodeCommand(X10Message *x10Message, unsigned char *encodedMessage) {
                //char encodedMessage[MAX_CMD_LEN];
                int msgLen;

                memset(encodedMessage, 0, MAX_CMD_LEN);

                if (IS_CAM_CODE(x10Message->command)) {
                    msgLen = CAM_CMD_LEN;

                    encodedMessage[0] = CAM_CMD_PFX;
                    encodedMessage[1] = (x10Message->command >> 8) | house_code_to_cam_code(x10Message->houseCode);
                    encodedMessage[2] = x10Message->command & 0xFF;
                    encodedMessage[3] = x10Message->houseCode;
                } else {
                    msgLen = NORM_CMD_LEN;

                    encodedMessage[0] = NORM_CMD_PFX;
                    encodedMessage[1] = (x10Message->unitCode >> 8) | x10Message->houseCode;
                    encodedMessage[2] = ~encodedMessage[1];
                    encodedMessage[3] = (x10Message->unitCode & 0xFF) | x10Message->command;
                    encodedMessage[4] = ~encodedMessage[3];
                }

                return msgLen;
            }

            int X10Message::decodeCommand(unsigned char *encodedMessage, X10Message *decodedMessage) {
                int cmdLen = 0;

                switch (encodedMessage[0]) {
                    case CAM_CMD_PFX:
                        cmdLen = CAM_CMD_LEN;
                        //debug(DBG_INFO, "Read camera command prefix\n");
                        break;
                    case NORM_CMD_PFX:
                        cmdLen = NORM_CMD_LEN;
                        //debug(DBG_INFO, "Read on/off/brighten/dim command prefix\n");
                        break;
                    default:
                        //error("%s: Read invalid command prefix: %02X\n", __FUNCTION__, x10Dev->intrInBuf[0]);
                        return -1;
                }

                if (cmdLen == CAM_CMD_LEN) {
                    decodedMessage->command = (enum Command) (((encodedMessage[1] & 0xF) << 8) | encodedMessage[2]);
                    decodedMessage->houseCode = (enum HouseCode) encodedMessage[3];
                    decodedMessage->unitCode = UNIT_1;
                } else {
                    decodedMessage->unitCode = (enum UnitCode) (((encodedMessage[1] & 0xF) << 8) | (encodedMessage[3] & ~CMD_OFF));
                    decodedMessage->houseCode = (enum HouseCode) (encodedMessage[1] & 0xF0);
                    decodedMessage->command = (enum Command) (encodedMessage[3] & CMD_OFF);
                }

                return 0;
            }

    }
}
