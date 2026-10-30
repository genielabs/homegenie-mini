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
#ifndef HOMEGENIE_MINI_FX_SCRIPT_TYPES_H
#define HOMEGENIE_MINI_FX_SCRIPT_TYPES_H

#include <vector>
#include <algorithm>

#include "../configuration.h"

enum CmdType : uint8_t {
    CMD_OFF = 0,
    CMD_ON,
    CMD_TOGGLE,
    CMD_LEVEL,
    CMD_HSB,
    CMD_JUMP,          // For #LOOP
    CMD_JUMP_REPEAT,   // For #REPEAT
    CMD_DEFAULT        // Show `defaultColor`
};

// Temporary structure used only during compilation (discarded immediately after)
struct TempInstruction {
    uint32_t timestamp;
    uint16_t ledStart;
    uint16_t ledEnd;
    uint8_t type;
    uint8_t h;
    uint8_t s;
    uint8_t v;
    uint16_t transition;
    uint8_t labelNum;
    uint8_t repeatCount;
};

// Byte serialization helpers to ensure portability and alignment safety
inline void write8(std::vector<uint8_t>& buf, uint8_t val) {
    buf.push_back(val);
}

inline void write16(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
}

inline void write32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

inline void write16At(std::vector<uint8_t>& buf, size_t offset, uint16_t val) {
    buf[offset] = val & 0xFF;
    buf[offset + 1] = (val >> 8) & 0xFF;
}

inline void write32At(std::vector<uint8_t>& buf, size_t offset, uint32_t val) {
    buf[offset] = val & 0xFF;
    buf[offset + 1] = (val >> 8) & 0xFF;
    buf[offset + 2] = (val >> 16) & 0xFF;
    buf[offset + 3] = (val >> 24) & 0xFF;
}

// Compile a logical rectangle along the physical major axis. Each span is
// contiguous even with vertical/serpentine wiring; no per-frame remap or buffer.
inline void appendScriptRectangle(std::vector<TempInstruction>& instructions,
                                  const MatrixConfig& cfg, const TempInstruction& command,
                                  uint16_t colStart, uint16_t rowStart,
                                  uint16_t colEnd, uint16_t rowEnd) {
    if (cfg.cols == 0 || cfg.rows == 0 || colStart >= cfg.cols || rowStart >= cfg.rows
        || colEnd < colStart || rowEnd < rowStart) return;
    if (colEnd >= cfg.cols) colEnd = cfg.cols - 1;
    if (rowEnd >= cfg.rows) rowEnd = cfg.rows - 1;
    const uint16_t majorStart = cfg.isVertical ? colStart : rowStart;
    const uint16_t majorEnd = cfg.isVertical ? colEnd : rowEnd;
    const uint16_t minorStart = cfg.isVertical ? rowStart : colStart;
    const uint16_t minorEnd = cfg.isVertical ? rowEnd : colEnd;
    for (uint16_t major = majorStart; major <= majorEnd; ++major) {
        TempInstruction span = command;
        const uint16_t first = cfg.isVertical
            ? getLEDIndex(major, minorStart, cfg) : getLEDIndex(minorStart, major, cfg);
        const uint16_t last = cfg.isVertical
            ? getLEDIndex(major, minorEnd, cfg) : getLEDIndex(minorEnd, major, cfg);
        span.ledStart = first < last ? first : last;
        span.ledEnd = first < last ? last : first;
        if (span.ledStart >= cfg.count) continue;
        if (span.ledEnd >= cfg.count) span.ledEnd = cfg.count - 1;
        instructions.push_back(span);
    }
}

inline bool compileScript(const String& scriptText, std::vector<uint8_t>& compiledScript, const MatrixConfig& cfg) {
    compiledScript.clear();

    int startIdx = 0;

    // Explicitly strip leading '/plain/' or 'plain/' web-server router prefixes if present
    if (scriptText.startsWith("plain/")) {
        startIdx = 6;
    } else if (scriptText.startsWith("/plain/")) {
        startIdx = 7;
    }

    // Then find where the actual content begins, skipping leading whitespaces and newlines
    while (startIdx < scriptText.length() &&
          (scriptText[startIdx] == ' ' ||
           scriptText[startIdx] == '\r' || scriptText[startIdx] == '\n')) {
        startIdx++;
    }

    if (startIdx >= scriptText.length()) return false;

    std::vector<TempInstruction> tempInstructions;
    // Estimate lines to avoid small heap reallocations
    int lineCount = 0;
    int idx = startIdx;
    while ((idx = scriptText.indexOf('\n', idx)) != -1) {
        lineCount++;
        idx++;
    }
    tempInstructions.reserve(lineCount + 2);

    const uint8_t MAX_LABELS = 16;
    int labelToTempInstIndex[MAX_LABELS];
    memset(labelToTempInstIndex, -1, sizeof(labelToTempInstIndex));

    uint32_t currentParsedTime = 0;
    int currentPos = startIdx;
    int parsedLinesCount = 0;
    bool recordingLabel[MAX_LABELS] = {false};

    // PASS 1: Parse the text script into a temporary instructions vector
    while (currentPos < scriptText.length()) {
        if (parsedLinesCount++ % 50 == 0) {
            yield();
        }

        int lineEnd = scriptText.indexOf('\n', currentPos);
        if (lineEnd == -1) lineEnd = scriptText.length();

        String line = scriptText.substring(currentPos, lineEnd);
        line.trim();

        if (line.length() == 0 || line.startsWith("//")) {
            currentPos = lineEnd + 1;
            continue;
        }

        if (line.startsWith("#")) {
            String cmd = line;
            cmd.toUpperCase();
            cmd.replace(" ", "");

            if (cmd.startsWith("#REPEAT#")) {
                int hashIdx = cmd.indexOf('#', 7);
                if (hashIdx > 0) {
                    uint8_t labelNum = cmd.substring(hashIdx + 1, hashIdx + 2).toInt();
                    uint8_t count = cmd.substring(hashIdx + 2).toInt();

                    if (labelNum < MAX_LABELS) {
                        TempInstruction inst;
                        inst.timestamp = currentParsedTime;
                        inst.type = CMD_JUMP_REPEAT;
                        inst.labelNum = labelNum;
                        inst.repeatCount = count;
                        tempInstructions.push_back(inst);
                    }
                }
            }
            else if (cmd.startsWith("#LOOP:")) {
                uint8_t labelNum = cmd.substring(6).toInt();
                if (labelNum < MAX_LABELS) {
                    TempInstruction inst;
                    inst.timestamp = currentParsedTime;
                    inst.type = CMD_JUMP;
                    inst.labelNum = labelNum;
                    tempInstructions.push_back(inst);
                }
            }
            else {
                uint8_t labelNum = line.substring(1).toInt();
                if (labelNum < MAX_LABELS) {
                    recordingLabel[labelNum] = true;
                }
            }
            currentPos = lineEnd + 1;
            continue;
        }

        int firstSpace = line.indexOf(' ');
        if (firstSpace > 0) {
            String timeStr = line.substring(0, firstSpace);

            // Count colons to identify the format (hh:mm:ss.mmm, mm:ss.mmm, or ss.mmm)
            int colonCount = 0;
            for (unsigned int c = 0; c < timeStr.length(); c++) {
                if (timeStr[c] == ':') {
                    colonCount++;
                }
            }

            int h = 0, m = 0, s = 0, ms = 0;
            bool timeParsed = false;

            if (colonCount == 2) {
                // Format: hh:mm:ss.mmm
                if (sscanf(timeStr.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms) == 4) {
                    timeParsed = true;
                }
            } else if (colonCount == 1) {
                // Format: mm:ss.mmm
                if (sscanf(timeStr.c_str(), "%d:%d.%d", &m, &s, &ms) == 3) {
                    h = 0;
                    timeParsed = true;
                }
            } else if (colonCount == 0) {
                // Format: ss.mmm
                if (sscanf(timeStr.c_str(), "%d.%d", &s, &ms) == 2) {
                    h = 0;
                    m = 0;
                    timeParsed = true;
                }
            }

            if (timeParsed) {
                currentParsedTime = (h * 3600000) + (m * 60000) + (s * 1000) + ms;

                for (uint8_t l = 0; l < MAX_LABELS; l++) {
                    if (recordingLabel[l]) {
                        labelToTempInstIndex[l] = tempInstructions.size();
                        recordingLabel[l] = false;
                    }
                }

                int secondSpace = line.indexOf(' ', firstSpace + 1);
                if (secondSpace > 0) {
                    String ledStr = line.substring(firstSpace + 1, secondSpace);
                    ledStr.toUpperCase(); // Make LED range parsing case-insensitive

                    // Extract and prepare the command string
                    String cmdStr = line.substring(secondSpace + 1);
                    cmdStr.trim();
                    cmdStr.toUpperCase();
                    cmdStr.replace(" ", "");

                    // Parse the command payload first
                    uint8_t cmdType = 0;
                    uint8_t cmdH = 0, cmdS = 0, cmdV = 0;
                    uint16_t transition = 0;
                    uint8_t labelNum = 0;
                    uint8_t repeatCount = 0;

                    if (cmdStr == "ON") {
                        cmdType = CMD_ON;
                    } else if (cmdStr == "OFF") {
                        cmdType = CMD_OFF;
                    } else if (cmdStr == "TOGGLE") {
                        cmdType = CMD_TOGGLE;
                    } else if (cmdStr == "DEFAULT") {
                        cmdType = CMD_DEFAULT;
                        cmdV = 255; // Default to 100% of master brightness
                        transition = 0;
                    } else if (cmdStr.startsWith("DEFAULT(")) {
                        cmdType = CMD_DEFAULT;
                        String args = cmdStr.substring(cmdStr.indexOf('(') + 1, cmdStr.lastIndexOf(')'));
                        int comma = args.indexOf(',');
                        float lvl = 1.0f;
                        int tr = 0;
                        if (comma > 0) {
                            // Format: DEFAULT(value, transition) -> e.g. DEFAULT(0.10, 1500)
                            lvl = args.substring(0, comma).toFloat();
                            tr = args.substring(comma + 1).toInt();
                        } else {
                            // Format: DEFAULT(value) or DEFAULT(transition)
                            if (args.indexOf('.') >= 0 || args.toFloat() <= 1.0f) {
                                lvl = args.toFloat();
                                tr = 0;
                            } else {
                                lvl = 1.0f;
                                tr = args.toInt();
                            }
                        }
                        cmdV = (uint8_t)(lvl * 255.0f);
                        transition = tr;
                    } else if (cmdStr.startsWith("LEVEL(")) {
                        cmdType = CMD_LEVEL;
                        String args = cmdStr.substring(cmdStr.indexOf('(') + 1, cmdStr.lastIndexOf(')'));
                        int comma = args.indexOf(',');
                        float lvl = 0.0f;
                        if (comma > 0) {
                            lvl = args.substring(0, comma).toFloat() / 100.0f; // Added () to toFloat
                            transition = args.substring(comma + 1).toInt();
                        } else {
                            lvl = args.toFloat() / 100.0f;
                        }
                        cmdV = (uint8_t)(lvl * 255.0f);
                    } else if (cmdStr.startsWith("HSB(")) {
                        cmdType = CMD_HSB;
                        String args = cmdStr.substring(cmdStr.indexOf('(') + 1, cmdStr.lastIndexOf(')'));
                        int c1 = args.indexOf(',');
                        int c2 = args.indexOf(',', c1 + 1);
                        int c3 = args.indexOf(',', c2 + 1);

                        if (c1 > 0 && c2 > 0 && c3 > 0) {
                            float h_val = args.substring(0, c1).toFloat();
                            float s_val = args.substring(c1 + 1, c2).toFloat();
                            float b_val = args.substring(c2 + 1, c3).toFloat();
                            transition = args.substring(c3 + 1).toInt();

                            cmdH = (uint8_t)(h_val * 255.0f);
                            cmdS = (uint8_t)(s_val * 255.0f);
                            cmdV = (uint8_t)(b_val * 255.0f);
                        }
                    } else {
                        currentPos = lineEnd + 1;
                        continue;
                    }

                    // Process LED ranges and push instructions dynamically
                    if (ledStr == "ALL") {
                        TempInstruction inst;
                        inst.timestamp = currentParsedTime;
                        inst.ledStart = 0;
                        inst.ledEnd = 0xFFFF;
                        inst.type = cmdType;
                        inst.h = cmdH; inst.s = cmdS; inst.v = cmdV;
                        inst.transition = transition;
                        inst.labelNum = labelNum;
                        inst.repeatCount = repeatCount;
                        tempInstructions.push_back(inst);
                    } else {
                        int plusIdx = ledStr.indexOf('+');
                        if (plusIdx > 0) {
                            // Handler for relative offsets '+'
                            String startStr = ledStr.substring(0, plusIdx);
                            String offsetStr = ledStr.substring(plusIdx + 1);

                            int colonStart = startStr.indexOf(':');
                            if (colonStart > 0) {
                                // 2D start coordinate
                                uint16_t rowStart = startStr.substring(0, colonStart).toInt();
                                uint16_t colStart = startStr.substring(colonStart + 1).toInt();

                                int colonOffset = offsetStr.indexOf(':');
                                if (colonOffset > 0) {
                                    // 2D Rectangle Box: +width:height
                                    uint16_t width = offsetStr.substring(0, colonOffset).toInt();
                                    uint16_t height = offsetStr.substring(colonOffset + 1).toInt();

                                    if (width == 0 || height == 0) {
                                        currentPos = lineEnd + 1;
                                        continue;
                                    }
                                    TempInstruction inst{};
                                    inst.timestamp = currentParsedTime;
                                    inst.type = cmdType;
                                    inst.h = cmdH; inst.s = cmdS; inst.v = cmdV;
                                    inst.transition = transition;
                                    inst.labelNum = labelNum;
                                    inst.repeatCount = repeatCount;
                                    appendScriptRectangle(tempInstructions, cfg, inst,
                                        colStart, rowStart,
                                        (uint16_t)std::min<uint32_t>((uint32_t)colStart + width - 1, UINT16_MAX),
                                        (uint16_t)std::min<uint32_t>((uint32_t)rowStart + height - 1, UINT16_MAX));
                                } else {
                                    // 2D horizontal span on the same row: +width
                                    uint16_t width = offsetStr.toInt();
                                    if (width == 0) {
                                        currentPos = lineEnd + 1;
                                        continue;
                                    }
                                    TempInstruction inst{};
                                    inst.timestamp = currentParsedTime;
                                    inst.type = cmdType;
                                    inst.h = cmdH; inst.s = cmdS; inst.v = cmdV;
                                    inst.transition = transition;
                                    inst.labelNum = labelNum;
                                    inst.repeatCount = repeatCount;
                                    appendScriptRectangle(tempInstructions, cfg, inst,
                                        colStart, rowStart,
                                        (uint16_t)std::min<uint32_t>((uint32_t)colStart + width - 1, UINT16_MAX), rowStart);
                                }
                            } else {
                                // 1D Offset: start+length
                                uint16_t start = startStr.toInt();
                                uint16_t length = offsetStr.toInt();

                                TempInstruction inst;
                                inst.timestamp = currentParsedTime;
                                inst.ledStart = start;
                                inst.ledEnd = start + length - 1;
                                inst.type = cmdType;
                                inst.h = cmdH; inst.s = cmdS; inst.v = cmdV;
                                inst.transition = transition;
                                inst.labelNum = labelNum;
                                inst.repeatCount = repeatCount;
                                tempInstructions.push_back(inst);
                            }
                        } else {
                            // Handler for standard '-' ranges
                            int dashIdx = ledStr.indexOf('-');
                            if (dashIdx > 0) {
                                String startStr = ledStr.substring(0, dashIdx);
                                String endStr = ledStr.substring(dashIdx + 1);

                                int colon1 = startStr.indexOf(':');
                                int colon2 = endStr.indexOf(':');

                                if (colon1 > 0 && colon2 > 0) {
                                    // 2D Area bounding box: rowStart:colStart - rowEnd:colEnd
                                    uint16_t r1 = startStr.substring(0, colon1).toInt();
                                    uint16_t c1 = startStr.substring(colon1 + 1).toInt();
                                    uint16_t r2 = endStr.substring(0, colon2).toInt();
                                    uint16_t c2 = endStr.substring(colon2 + 1).toInt();

                                    TempInstruction inst{};
                                    inst.timestamp = currentParsedTime;
                                    inst.type = cmdType;
                                    inst.h = cmdH; inst.s = cmdS; inst.v = cmdV;
                                    inst.transition = transition;
                                    inst.labelNum = labelNum;
                                    inst.repeatCount = repeatCount;
                                    appendScriptRectangle(tempInstructions, cfg, inst,
                                        std::min(c1, c2), std::min(r1, r2),
                                        std::max(c1, c2), std::max(r1, r2));
                                } else {
                                    // 1D or Mixed Range
                                    uint16_t ledStart = 0;
                                    uint16_t ledEnd = 0;

                                    if (colon1 > 0) {
                                        uint16_t r1 = startStr.substring(0, colon1).toInt();
                                        uint16_t c1 = startStr.substring(colon1 + 1).toInt();
                                        ledStart = getLEDIndex(c1, r1, cfg);
                                    } else {
                                        ledStart = startStr.toInt();
                                    }

                                    if (colon2 > 0) {
                                        uint16_t r2 = endStr.substring(0, colon2).toInt();
                                        uint16_t c2 = endStr.substring(colon2 + 1).toInt();
                                        ledEnd = getLEDIndex(c2, r2, cfg);
                                    } else {
                                        ledEnd = endStr.toInt();
                                    }

                                    TempInstruction inst;
                                    inst.timestamp = currentParsedTime;
                                    inst.ledStart = ledStart;
                                    inst.ledEnd = ledEnd;
                                    inst.type = cmdType;
                                    inst.h = cmdH; inst.s = cmdS; inst.v = cmdV;
                                    inst.transition = transition;
                                    inst.labelNum = labelNum;
                                    inst.repeatCount = repeatCount;
                                    tempInstructions.push_back(inst);
                                }
                            } else {
                                // Single LED index or Single 2D coordinate
                                uint16_t ledStart = 0;
                                int colonStart = ledStr.indexOf(':');
                                if (colonStart > 0) {
                                    uint16_t r = ledStr.substring(0, colonStart).toInt();
                                    uint16_t c = ledStr.substring(colonStart + 1).toInt();
                                    ledStart = getLEDIndex(c, r, cfg);
                                } else {
                                    ledStart = ledStr.toInt();
                                }

                                TempInstruction inst;
                                inst.timestamp = currentParsedTime;
                                inst.ledStart = ledStart;
                                inst.ledEnd = ledStart;
                                inst.type = cmdType;
                                inst.h = cmdH; inst.s = cmdS; inst.v = cmdV;
                                inst.transition = transition;
                                inst.labelNum = labelNum;
                                inst.repeatCount = repeatCount;
                                tempInstructions.push_back(inst);
                            }
                        }
                    }
                    currentPos = lineEnd + 1;
                    continue;
                }
            }
        }
        currentPos = lineEnd + 1;
    }

    if (tempInstructions.empty()) return false;

    // PASS 2: Group by timestamp into Frame Blocks and write variable-length bytecode
    struct JumpPatch {
        size_t patchOffset;
        uint8_t labelNum;
    };
    std::vector<JumpPatch> jumpPatches;

    int labelByteOffsets[MAX_LABELS];
    uint32_t labelByteTimes[MAX_LABELS];
    memset(labelByteOffsets, -1, sizeof(labelByteOffsets));
    memset(labelByteTimes, 0, sizeof(labelByteTimes));

    size_t i = 0;
    while (i < tempInstructions.size()) {
        uint32_t frameTime = tempInstructions[i].timestamp;
        size_t frameStartInstIdx = i;
        size_t frameCmdCount = 0;

        while (i + frameCmdCount < tempInstructions.size() &&
               tempInstructions[i + frameCmdCount].timestamp == frameTime) {
            frameCmdCount++;
        }

        size_t frameByteOffset = compiledScript.size();

        for (uint8_t l = 0; l < MAX_LABELS; l++) {
            if (labelToTempInstIndex[l] >= 0 &&
                (size_t)labelToTempInstIndex[l] >= frameStartInstIdx &&
                (size_t)labelToTempInstIndex[l] < frameStartInstIdx + frameCmdCount) {
                if (labelByteOffsets[l] == -1) {
                    labelByteOffsets[l] = frameByteOffset;
                    labelByteTimes[l] = frameTime;
                }
            }
        }

        write32(compiledScript, frameTime);
        write16(compiledScript, frameCmdCount);

        for (size_t c = 0; c < frameCmdCount; c++) {
            size_t currentInstIdx = frameStartInstIdx + c;
            const TempInstruction& inst = tempInstructions[currentInstIdx];

            write8(compiledScript, inst.type);

            if (inst.type == CMD_ON || inst.type == CMD_OFF || inst.type == CMD_TOGGLE) {
                write16(compiledScript, inst.ledStart);
                write16(compiledScript, inst.ledEnd);
            }
            else if (inst.type == CMD_LEVEL) {
                write16(compiledScript, inst.ledStart);
                write16(compiledScript, inst.ledEnd);
                write8(compiledScript, inst.v);
                write16(compiledScript, inst.transition);
            }
            else if (inst.type == CMD_HSB) {
                write16(compiledScript, inst.ledStart);
                write16(compiledScript, inst.ledEnd);
                write8(compiledScript, inst.h);
                write8(compiledScript, inst.s);
                write8(compiledScript, inst.v);
                write16(compiledScript, inst.transition);
            }
            else if (inst.type == CMD_JUMP) {
                write8(compiledScript, inst.labelNum);
                size_t patchPos = compiledScript.size();
                write16(compiledScript, 0);
                write32(compiledScript, 0);
                jumpPatches.push_back({patchPos, inst.labelNum});
            }
            else if (inst.type == CMD_JUMP_REPEAT) {
                write8(compiledScript, inst.labelNum);
                write8(compiledScript, inst.repeatCount);
                size_t patchPos = compiledScript.size();
                write16(compiledScript, 0);
                write32(compiledScript, 0);
                jumpPatches.push_back({patchPos, inst.labelNum});
            }
            else if (inst.type == CMD_DEFAULT) {
                write16(compiledScript, inst.ledStart);
                write16(compiledScript, inst.ledEnd);
                write8(compiledScript, inst.v); // Write the custom brightness byte
                write16(compiledScript, inst.transition);
            }
        }

        i += frameCmdCount;
    }

    for (const auto& patch : jumpPatches) {
        int targetOffset = labelByteOffsets[patch.labelNum];
        uint32_t targetTime = labelByteTimes[patch.labelNum];
        if (targetOffset >= 0) {
            write16At(compiledScript, patch.patchOffset, (uint16_t)targetOffset);
            write32At(compiledScript, patch.patchOffset + 2, targetTime);
        }
    }

    std::vector<uint8_t>(compiledScript).swap(compiledScript);
    return true;
}

#endif //HOMEGENIE_MINI_FX_SCRIPT_TYPES_H
