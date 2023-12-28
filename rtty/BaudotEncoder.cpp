/*
Copyright (C) 2023 - Bruce MacKinnon KC1FSZ

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free 
Software Foundation, either version 3 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "BaudotEncoder.h"
#include "BaudotDecoder.h"

using namespace std;

namespace radlib {

static void sendBaudotChar(FSKModulator& mod, uint32_t symbolLengthUs, uint8_t ch) {
    // Start bit
    mod.sendSpace(symbolLengthUs);
    // 5 data bits, sendind the MSB first.
    for (uint8_t i = 0; i < 5; i++) {
        if (ch & 0b10000) {
            mod.sendMark(symbolLengthUs);
        } else {
            mod.sendSpace(symbolLengthUs);
        }
        ch = ch << 1;
    }
    // Stop bit
    mod.sendMark(symbolLengthUs + symbolLengthUs / 2);
}

void transmitBaudot(const char* msg, FSKModulator& mod, uint32_t symbolLengthUs) {

    // Send a short mark at the beginning so that we can see the 
    // transition of the initial start bit.    
    mod.sendMark(symbolLengthUs);
    mod.sendMark(symbolLengthUs);
    mod.sendMark(symbolLengthUs);
    mod.sendMark(symbolLengthUs);

    // We are assuming that the receiver always starts in LTRS mode
    BaudotMode mode = BaudotMode::LTRS;

    for (uint16_t i = 0; msg[i] != 0; i++) {
        // There are some special cases that can be handled without 
        // worrying about which mode we are in.
        if (msg[i] == '\n') {
            sendBaudotChar(mod, symbolLengthUs, 2);
        } else if (msg[i] == ' ') {
            sendBaudotChar(mod, symbolLengthUs, 4);
        } else if (msg[i] == '\r') {
            sendBaudotChar(mod, symbolLengthUs, 8);
        }
        else {
            // Scan the tables to find the ASCII character
            bool found = false;
            for (uint8_t k = 0; k < 2 && !found; k++) {
                for (uint8_t b = 0; b < 32 && !found; b++) {
                    if (BAUDOT_TO_ASCII_MAP[b][k] == msg[i]) {
                        found = true;
                        // Figure out if we need to change modes
                        if (mode == BaudotMode::LTRS && k == 1) {
                            sendBaudotChar(mod, symbolLengthUs, BAUDOT_FIGS);
                            mode = BaudotMode::FIGS;
                        } else if (mode == BaudotMode::FIGS && k == 0) {
                            sendBaudotChar(mod, symbolLengthUs, BAUDOT_LTRS);
                            mode = BaudotMode::LTRS;
                        }
                        sendBaudotChar(mod, symbolLengthUs, b);
                    }
                }
            }
        }
    }

    // Make sure we leave the receiver in letters mode
    if (mode == BaudotMode::FIGS) {
        sendBaudotChar(mod, symbolLengthUs, BAUDOT_LTRS);
    }
}

}

