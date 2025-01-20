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

#ifndef HOMEGENIE_MINI_X10_X10_MESSAGE_H_
#define HOMEGENIE_MINI_X10_X10_MESSAGE_H_

#include <Config.h>

/* Maximum debugging level to actually display */
#define DBG_MAX DBG_INFO
#define MSG_PFX "x10"
#define DEBUG 1

#if DEBUG
#define debug(sev, format, args...)                    \
    do { printk(MSG_PFX format, ## args); } while (0)
#else
#define debug(sev, format, args...)					\
//     do { if (sev <= DBG_MAX) printk(KERN_DEBUG MSG_PFX format, ## args); } while (0)
#endif
// #define error(format, args...) printk(KERN_ERR MSG_PFX format, ## args)
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

#define IS_CAM_CODE(code) ((code) & ~0xFF)

/** Normal command length */
#define NORM_CMD_LEN 5
/** Pan'n'Tilt command length */
#define CAM_CMD_LEN 4
/** Larger of the two lengths, used for allocating buffers */
#define MAX_CMD_LEN NORM_CMD_LEN

/** Prefix for all normal commands */
#define NORM_CMD_PFX 0x20
/** Prefix for all Pan'n'Tilt commands */
#define CAM_CMD_PFX 0x14

#define ACK_LEN 1
#define ACK 0xFF

/* Bounds of X10 houseCode codes */
#define HOUSE_MIN 'a'
#define HOUSE_MAX 'p'

/* Bounds of X10 unitCode codes */
#define UNIT_MIN 1
#define UNIT_MAX 16

namespace IO {
    namespace X10 {

        enum HouseCode {
            HOUSE_A = 0x60, HOUSE_B = 0x70, HOUSE_C = 0x40, HOUSE_D = 0x50,
            HOUSE_E = 0x80, HOUSE_F = 0x90, HOUSE_G = 0xA0, HOUSE_H = 0xB0,
            HOUSE_I = 0xE0, HOUSE_J = 0xF0, HOUSE_K = 0xC0, HOUSE_L = 0xD0,
            HOUSE_M = 0x00, HOUSE_N = 0x10, HOUSE_O = 0x20, HOUSE_P = 0x30
        };

        static char house_code_to_char(enum HouseCode code) {
            switch (code) {
                case HOUSE_A:
                    return 'A';
                case HOUSE_B:
                    return 'B';
                case HOUSE_C:
                    return 'C';
                case HOUSE_D:
                    return 'D';
                case HOUSE_E:
                    return 'E';
                case HOUSE_F:
                    return 'F';
                case HOUSE_G:
                    return 'G';
                case HOUSE_H:
                    return 'H';
                case HOUSE_I:
                    return 'I';
                case HOUSE_J:
                    return 'J';
                case HOUSE_K:
                    return 'K';
                case HOUSE_L:
                    return 'L';
                case HOUSE_M:
                    return 'M';
                case HOUSE_N:
                    return 'N';
                case HOUSE_O:
                    return 'O';
                case HOUSE_P:
                    return 'P';
                default:
                    return '?';
            }
        }

        static enum HouseCode HouseCodeLut[(HOUSE_MAX - HOUSE_MIN) + 1] = {
                HOUSE_A, HOUSE_B, HOUSE_C, HOUSE_D, HOUSE_E, HOUSE_F, HOUSE_G, HOUSE_H,
                HOUSE_I, HOUSE_J, HOUSE_K, HOUSE_L, HOUSE_M, HOUSE_N, HOUSE_O, HOUSE_P
        };

/**
 * Translate house code to octet necessary for 2nd byte in Pan'n'Tilt commands
 */
        static uint8_t house_code_to_cam_code(enum HouseCode code) {
            switch (code) {
                case HOUSE_A:
                    return 0x90;
                case HOUSE_B:
                    return 0xA0;
                case HOUSE_C:
                    return 0x70;
                case HOUSE_D:
                    return 0x80;
                case HOUSE_E:
                    return 0xB0;
                case HOUSE_F:
                    return 0xC0;
                case HOUSE_G:
                    return 0xD0;
                case HOUSE_H:
                    return 0xE0;
                case HOUSE_I:
                    return 0x10;
                case HOUSE_J:
                    return 0x20;
                case HOUSE_K:
                    return 0xF0;
                case HOUSE_L:
                    return 0x00;
                case HOUSE_M:
                    return 0x30;
                case HOUSE_N:
                    return 0x40;
                case HOUSE_O:
                    return 0x50;
                case HOUSE_P:
                    return 0x60;
                default:
                    return 0x90; /* default to A if bad housecode */
            }
        }

/**
 * Unit codes
 */
        enum UnitCode {
            UNIT_1 = 0x000, UNIT_2 = 0x010, UNIT_3 = 0x008, UNIT_4 = 0x018,
            UNIT_5 = 0x040, UNIT_6 = 0x050, UNIT_7 = 0x048, UNIT_8 = 0x058,
            UNIT_9 = 0x400, UNIT_10 = 0x410, UNIT_11 = 0x408, UNIT_12 = 0x418,
            UNIT_13 = 0x440, UNIT_14 = 0x450, UNIT_15 = 0x448, UNIT_16 = 0x458,
            UNIT_NONE = 0xFFF
        };

        static enum UnitCode UnitCodeLut[(UNIT_MAX - UNIT_MIN) + 1] = {
                UNIT_1, UNIT_2, UNIT_3, UNIT_4, UNIT_5, UNIT_6, UNIT_7, UNIT_8,
                UNIT_9, UNIT_10, UNIT_11, UNIT_12, UNIT_13, UNIT_14, UNIT_15, UNIT_16
        };

        static int unit_code_to_int(enum UnitCode code) {
            if (code == UnitCode::UNIT_NONE) return 0;
            int unit = ((code >> 7) & 0x8);
            unit |= ((code >> 4) & 0x4);
            unit |= ((code >> 2) & 0x2);
            unit |= ((code >> 4) & 0x1);
            return ++unit;
        }

/**
 * Command codes
 */
        enum Command {
            /* Standard 5-byte commands: */
            CMD_ON = 0x00,  /* Turn on unitCode */
            CMD_OFF = 0x20,  /* Turn off unitCode */
            /* Lights */
            CMD_LIGHTS_ALL_OFF = 0x80,  /* Turn off unitCode */
            CMD_LIGHTS_ALL_ON = 0x90,  /* Turn off unitCode */
            CMD_LIGHTS_DIM = 0x98,  /* Dim lamp */
            CMD_LIGHTS_BRIGHT = 0x88,  /* Brighten lamp */
            /* Pan'n'Tilt 4-byte commands: */
            CMD_MOVE_UP = 0x762,
            CMD_MOVE_RIGHT = 0x661,
            CMD_MOVE_DOWN = 0x863,
            CMD_MOVE_LEFT = 0x560,
            /* Security codes */
            CMD_SEC_PANIC = 0x26D9,
            CMD_SEC_ARM_HOME = 0x0EF1,
            CMD_SEC_ARM_AWAY = 0x06F9,
            CMD_SEC_DISARM = 0x8679,
            CMD_SEC_LIGHT_OFF = 0xC639,
            CMD_SEC_LIGHT_ON = 0x46B9,
            CMD_SEC_MOTION_DETECTED = 0x0CF3,
            CMD_SEC_MOTION_IDLE = 0x8C73,
            CMD_SEC_MOTION_TAMPERED = 0x4CB3,
            CMD_SEC_DOOR_OPEN = 0x04FB,
            CMD_SEC_DOOR_CLOSED = 0x847B,
            CMD_SEC_DOOR_TAMPERED = 0x44BB,
            /* Error flag */
            CMD_INVALID = 0xFF
        };

        static enum Command parse_cmd_code(char c) {
            switch (c) {
                case '+':
                    return CMD_ON;
                case '-':
                    return CMD_OFF;
                case '.':
                    return CMD_LIGHTS_ALL_OFF;
                case '*':
                    return CMD_LIGHTS_ALL_ON;
                case '!':
                    return CMD_SEC_PANIC;
                case '#':
                    return CMD_SEC_ARM_HOME;
                case '%':
                    return CMD_SEC_ARM_AWAY;
                case '=':
                    return CMD_SEC_DISARM;
                case '/':
                    return CMD_SEC_LIGHT_OFF;
                case 'x':
                    return CMD_SEC_LIGHT_ON;
                case '$':
                    return CMD_SEC_MOTION_DETECTED;
                case ':':
                    return CMD_SEC_MOTION_IDLE;
                case '^':
                    return CMD_SEC_MOTION_TAMPERED;
                case '_':
                    return CMD_SEC_DOOR_OPEN;
                case 'H':
                    return CMD_SEC_DOOR_CLOSED;
                case 'X':
                    return CMD_SEC_DOOR_TAMPERED;
                case 'u':
                    return CMD_MOVE_UP;
                case 'd':
                    return CMD_MOVE_DOWN;
                case 'l':
                    return CMD_MOVE_LEFT;
                case 'r':
                    return CMD_MOVE_RIGHT;
                case 'b':
                    return CMD_LIGHTS_BRIGHT;
                case 's':
                    return CMD_LIGHTS_DIM;
                default:
                    //error("%s: Invalid command code: %c\n", __FUNCTION__, c);
                    return CMD_INVALID;
            }
        }

        static char cmd_code_to_char(enum Command code) {
            switch (code) {
                case CMD_ON:
                    return '+';
                case CMD_OFF:
                    return '-';
                case CMD_LIGHTS_ALL_OFF:
                    return '.';
                case CMD_LIGHTS_ALL_ON:
                    return '*';
                case CMD_LIGHTS_DIM:
                    return 's';
                case CMD_LIGHTS_BRIGHT:
                    return 'b';
                case CMD_SEC_PANIC:
                    return '!';
                case CMD_SEC_ARM_HOME:
                    return '#';
                case CMD_SEC_ARM_AWAY:
                    return '%';
                case CMD_SEC_DISARM:
                    return '=';
                case CMD_SEC_LIGHT_OFF:
                    return '/';
                case CMD_SEC_LIGHT_ON:
                    return 'x';
                case CMD_SEC_MOTION_DETECTED:
                    return '$';
                case CMD_SEC_MOTION_IDLE:
                    return ':';
                case CMD_SEC_MOTION_TAMPERED:
                    return '^';
                case CMD_SEC_DOOR_OPEN:
                    return '_';
                case CMD_SEC_DOOR_CLOSED:
                    return 'H';
                case CMD_SEC_DOOR_TAMPERED:
                    return 'X';
                case CMD_MOVE_UP:
                    return 'u';
                case CMD_MOVE_RIGHT:
                    return 'r';
                case CMD_MOVE_DOWN:
                    return 'd';
                case CMD_MOVE_LEFT:
                    return 'l';
                default:
                    return '?';
            }
        }

        static const char *cmd_code_to_str(enum Command code) {
            switch (code) {
                case CMD_ON:
                    return "on";
                case CMD_OFF:
                    return "off";
                case CMD_LIGHTS_ALL_OFF:
                    return "all_off";
                case CMD_LIGHTS_ALL_ON:
                    return "all_on";
                case CMD_LIGHTS_DIM:
                    return "dim";
                case CMD_LIGHTS_BRIGHT:
                    return "brighten";
                case CMD_SEC_PANIC:
                    return "panic";
                case CMD_SEC_ARM_HOME:
                    return "arm_home";
                case CMD_SEC_ARM_AWAY:
                    return "arm_away";
                case CMD_SEC_DISARM:
                    return "disarm";
                case CMD_SEC_LIGHT_OFF:
                    return "lights_off";
                case CMD_SEC_LIGHT_ON:
                    return "lights_on";
                case CMD_SEC_MOTION_DETECTED:
                    return "motion_detected";
                case CMD_SEC_MOTION_IDLE:
                    return "motion_idle";
                case CMD_SEC_MOTION_TAMPERED:
                    return "motion_tampered";
                case CMD_SEC_DOOR_OPEN:
                    return "door_open";
                case CMD_SEC_DOOR_CLOSED:
                    return "door_closed";
                case CMD_SEC_DOOR_TAMPERED:
                    return "door_tampered";
                case CMD_MOVE_UP:
                    return "up";
                case CMD_MOVE_RIGHT:
                    return "right";
                case CMD_MOVE_DOWN:
                    return "down";
                case CMD_MOVE_LEFT:
                    return "left";
                default:
                    return "invalid";
            }
        }

        class X10Message {
        public:

            /** Command code */
            enum Command command;
            /** House code */
            enum HouseCode houseCode;
            /** Unit code, 1-16 (not used for dim/bright or camera commands) */
            enum UnitCode unitCode;

            // static members
            static int encodeCommand(X10Message *x10Message, unsigned char *encodedMessage);
            static int decodeCommand(unsigned char *encodedMessage, X10Message *x10Message);

        };

    }
}

#endif // HOMEGENIE_MINI_IOMANAGER_H