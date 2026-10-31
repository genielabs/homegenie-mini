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
#ifndef HOMEGENIE_MINI_FX_SCRIPT_H
#define HOMEGENIE_MINI_FX_SCRIPT_H

#include "fx.h"
#include "fx_script_utils.h"

// Safe byte-by-byte deserializers to prevent alignment exceptions on ESP32/RISC-V
inline uint8_t read8(const std::vector<uint8_t>& buf, size_t& offset) {
    return buf[offset++];
}

inline uint16_t read16(const std::vector<uint8_t>& buf, size_t& offset) {
    uint16_t val = buf[offset++];
    val |= (uint16_t)buf[offset++] << 8;
    return val;
}

inline uint32_t read32(const std::vector<uint8_t>& buf, size_t& offset) {
    uint32_t val = buf[offset++];
    val |= (uint32_t)buf[offset++] << 8;
    val |= (uint32_t)buf[offset++] << 16;
    val |= (uint32_t)buf[offset++] << 24;
    return val;
}

// Modular helper to execute target color and levels on a given LED range
inline void executeOnLedRange(uint16_t ledStart, uint16_t ledEnd, uint8_t type, const LightColor& defaultColor,
                              uint8_t h_byte, uint8_t s_byte, uint8_t v_byte, uint16_t transition, uint16_t ledsCount) {
    if (ledsCount == 0) return;
    int start = ledStart;
    int end = ledEnd;

    // Resolve ALL dynamically. Normalize explicit ranges before clipping so a
    // range entirely beyond a shrunken strip cannot swap back out of bounds.
    if (end == 0xFFFF) {
        start = 0;
        end = ledsCount - 1;
    } else {
        if (start > end) std::swap(start, end);
        if (start >= ledsCount) return;
        if (end >= ledsCount) end = ledsCount - 1;
    }

    if (start <= end) {
        float defH = defaultColor.getHue();
        float defS = defaultColor.getSaturation();
        float defV = defaultColor.getValue(); // Master brightness (0.0 to 1.0)

        for (int i = start; i <= end; i++) {
            if (animatedColors[i] == nullptr) {
                continue;
            }

            switch (type) {
                case CMD_ON:
                    animatedColors[i]->setColor(defH, defS, defV, 0);
                    break;
                case CMD_OFF:
                    animatedColors[i]->setColor(defH, defS, 0, 0);
                    break;
                case CMD_DEFAULT: {
                    float lvl = v_byte / 255.0f;
                    if (lvl < 0) lvl = 0;
                    if (lvl > 1.0f) lvl = 1.0f;
                    float actualLvl = lvl * defV; // Scale custom brightness with master brightness
                    animatedColors[i]->setColor(defH, defS, actualLvl, transition);
                    break;
                }
                case CMD_TOGGLE:
                    if (animatedColors[i]->getTargetValue() > 0) {
                        animatedColors[i]->setColor(defH, defS, 0, 0);
                    } else {
                        animatedColors[i]->setColor(defH, defS, defV, 0);
                    }
                    break;
                case CMD_LEVEL: {
                    float lvl = v_byte / 255.0f;
                    if (lvl < 0) lvl = 0;
                    if (lvl > 1.0f) lvl = 1.0f;
                    float actualLvl = lvl * defV;
                    animatedColors[i]->setColor(
                        animatedColors[i]->getTargetHue(),
                        animatedColors[i]->getTargetSaturation(),
                        actualLvl,
                        transition
                    );
                    break;
                }
                case CMD_HSB: {
                    float h = h_byte / 255.0f;
                    float s = s_byte / 255.0f;
                    float b = v_byte / 255.0f;
                    if (b < 0) b = 0;
                    if (b > 1.0f) b = 1.0f;
                    float actualB = b * defV;
                    animatedColors[i]->setColor(h, s, actualB, transition);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

// Thread-safe Bytecode execution player (Accepts the pre-compiled bytecode vector from the handler)
inline bool fx_script(const MatrixConfig& cfg, const LightColor& defaultColor, const std::vector<uint8_t>& compiledScript, uint32_t startTime) {
    static size_t currentByteOffset = 0;
    static uint32_t internalStartTime = 0;
    static uint32_t lastStartTime = 0;

    // Control flow tracking
    static uint32_t timeShift = 0;
    static uint32_t lastEventTime = 0;
    static uint8_t repeatCounts[16] = {0};

    // Reset state if a new script execution is triggered
    if (startTime != lastStartTime) {
        lastStartTime = startTime;
        internalStartTime = millis();
        currentByteOffset = 0;
        timeShift = 0;
        lastEventTime = 0;
        memset(repeatCounts, 0, sizeof(repeatCounts));
    }

    if (compiledScript.empty() || currentByteOffset >= compiledScript.size()) {
        return false;
    }

    uint32_t elapsed = millis() - internalStartTime;

    // Process and play Frame Blocks
    while (currentByteOffset < compiledScript.size()) {
        size_t tempOffset = currentByteOffset;
        uint32_t frameTime = read32(compiledScript, tempOffset);
        uint16_t frameCmdCount = read16(compiledScript, tempOffset);

        // Safety check: if we are lagging behind by more than 1 second (e.g. effect was paused/switched),
        // adjust timeShift to the current elapsed time to prevent an infinite catch-up loop.
        if (elapsed > (frameTime + timeShift) + 1000) {
            timeShift = elapsed - frameTime;
        }

        if (elapsed >= (frameTime + timeShift)) {
            currentByteOffset = tempOffset; // Advance past the Frame Header
            bool jumped = false;

            // Execute all commands packed in this Frame
            for (uint16_t c = 0; c < frameCmdCount; c++) {
                uint8_t type = read8(compiledScript, currentByteOffset);

                if (type == CMD_ON || type == CMD_OFF || type == CMD_TOGGLE) {
                    uint16_t ledStart = read16(compiledScript, currentByteOffset);
                    uint16_t ledEnd = read16(compiledScript, currentByteOffset);
                    executeOnLedRange(ledStart, ledEnd, type, defaultColor, 0, 0, 0, 0, cfg.count);
                }
                else if (type == CMD_DEFAULT) {
                    uint16_t ledStart = read16(compiledScript, currentByteOffset);
                    uint16_t ledEnd = read16(compiledScript, currentByteOffset);
                    uint8_t v = read8(compiledScript, currentByteOffset); // Read the custom brightness byte
                    uint16_t transition = read16(compiledScript, currentByteOffset);
                    executeOnLedRange(ledStart, ledEnd, type, defaultColor, 0, 0, v, transition, cfg.count);
                }
                else if (type == CMD_LEVEL) {
                    uint16_t ledStart = read16(compiledScript, currentByteOffset);
                    uint16_t ledEnd = read16(compiledScript, currentByteOffset);
                    uint8_t v = read8(compiledScript, currentByteOffset);
                    uint16_t transition = read16(compiledScript, currentByteOffset);
                    executeOnLedRange(ledStart, ledEnd, type, defaultColor, 0, 0, v, transition, cfg.count);
                }
                else if (type == CMD_HSB) {
                    uint16_t ledStart = read16(compiledScript, currentByteOffset);
                    uint16_t ledEnd = read16(compiledScript, currentByteOffset);
                    uint8_t h = read8(compiledScript, currentByteOffset);
                    uint8_t s = read8(compiledScript, currentByteOffset);
                    uint8_t v = read8(compiledScript, currentByteOffset);
                    uint16_t transition = read16(compiledScript, currentByteOffset);
                    executeOnLedRange(ledStart, ledEnd, type, defaultColor, h, s, v, transition, cfg.count);
                }
                else if (type == CMD_JUMP) {
                    uint8_t labelNum = read8(compiledScript, currentByteOffset);
                    uint16_t jumpTarget = read16(compiledScript, currentByteOffset);
                    uint32_t labelTime = read32(compiledScript, currentByteOffset);

                    // Fixed: use frameTime instead of lastEventTime to calculate blockDuration accurately
                    uint32_t blockDuration = (frameTime > labelTime) ? (frameTime - labelTime) : 50;
                    timeShift += blockDuration;
                    currentByteOffset = jumpTarget;
                    jumped = true;
                    break; // Break sub-command loop to re-evaluate at the new bytecode offset
                }
                else if (type == CMD_JUMP_REPEAT) {
                    uint8_t labelNum = read8(compiledScript, currentByteOffset);
                    uint8_t repeatCount = read8(compiledScript, currentByteOffset);
                    uint16_t jumpTarget = read16(compiledScript, currentByteOffset);
                    uint32_t labelTime = read32(compiledScript, currentByteOffset);

                    if (repeatCounts[labelNum] < repeatCount - 1) {
                        repeatCounts[labelNum]++;
                        // Fixed: use frameTime instead of lastEventTime to calculate blockDuration accurately
                        uint32_t blockDuration = (frameTime > labelTime) ? (frameTime - labelTime) : 50;
                        timeShift += blockDuration;
                        currentByteOffset = jumpTarget;
                    } else {
                        repeatCounts[labelNum] = 0;
                    }
                    jumped = true;
                    break; // Break to re-evaluate
                }
            }

            if (!jumped) {
                lastEventTime = frameTime;
            }
        } else {
            break; // Timestamp not reached, wait for next tick
        }
    }

    return true;
}

#endif // HOMEGENIE_MINI_FX_SCRIPT_H
