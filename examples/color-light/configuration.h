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

// Limits LED strip power (percent [5% .. 100%])
#pragma once

#define DEFAULT_MAX_POWER 25

#define BUTTON_COMMAND_STEP_INTERVAL 200

#define BUTTON_COMMAND_NONE 0
#define BUTTON_COMMAND_STEP_BRIGHT 1
#define BUTTON_COMMAND_STEP_DIM 2
#define BUTTON_COMMAND_PRESET_COLORS 3

// Configuration structure for any matrix layout
struct MatrixConfig {
    uint16_t count;     // the total number of LEDs
    uint16_t cols;      // this is calculated as count/rows
    uint16_t rows;      // 1 = LED strip, >1 LED matrix
    bool isVertical;    // true = Column-Major, false = Row-Major
    bool isSerpentine;  // true = Zigzag, false = Progressive
    bool startAtBottom; // true = Origin at bottom, false = Origin at top
    bool startAtRight;  // true = Origin at right, false = Origin at left
};

// Universal XY Mapper
inline uint16_t getLEDIndex(uint16_t x, uint16_t y, const MatrixConfig& cfg) {
    uint16_t physX = x;
    uint16_t physY = y;

    // 1. Handle Horizontal/Vertical Origin Flips
    if (cfg.startAtRight) {
        physX = cfg.cols - 1 - x;
    }
    if (cfg.startAtBottom) {
        physY = cfg.rows - 1 - y;
    }

    // 2. Map coordinates to 1D Index based on Major Axis and Cablaging Style
    if (cfg.isVertical) {
        // Vertical columns first
        if (cfg.isSerpentine && (physX % 2 != 0)) {
            return (physX * cfg.rows) + (cfg.rows - 1 - physY);
        } else {
            return (physX * cfg.rows) + physY;
        }
    } else {
        // Horizontal rows first
        if (cfg.isSerpentine && (physY % 2 != 0)) {
            return (physY * cfg.cols) + (cfg.cols - 1 - physX);
        } else {
            return (physY * cfg.cols) + physX;
        }
    }
}
