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
#include <iostream>
#include "BaudotDecoder.h"

using namespace std;
using namespace radlib;

namespace radlib {

const uint8_t BAUDOT_LTRS = 31;
const uint8_t BAUDOT_FIGS = 27;

const char BAUDOT_TO_ASCII_MAP[32][2] = {
    {   0, 0   },
    { 'E', '3' },
    { '\n', '\n' },
    { 'A', '-' },
    { ' ', ' ' },
    { 'S', '\x07' },
    { 'I', '8' },
    { 'U', '7' },
    { '\r', '\r' },
    { 'D', '$' },
    { 'R', '4' },
    { 'J', '\'' },
    { 'N', ',' },
    { 'F', '!' },
    { 'C', ':' },
    { 'K', '(' },
    { 'T', '5' },
    { 'Z', '"' },
    { 'L', ')' },
    { 'W', '2' },
    { 'H', '#' },
    { 'Y', '6' },
    { 'P', '0' },
    { 'Q', '1' },
    { 'O', '9' },
    { 'B', '?' },
    { 'G', '&' },
    {   0, 0   },    // FIGS
    { 'M', '.' },
    { 'X', '/' },
    { 'V', ';' },
    {   0, 0,  }    // LTRS
};

BaudotDecoder::BaudotDecoder(uint16_t sampleRate, uint16_t baudRateTimes100,
    uint16_t windowSizeLog2, int16_t* windowArea) 
:   _samplesPerSymbol((100 * sampleRate) / baudRateTimes100),
    _mode(BaudotMode::LTRS),
    _avg(windowSizeLog2, windowArea),
    _state(0) {
}

void BaudotDecoder::reset() {
    _mode = BaudotMode::LTRS;
    _state = 0;
    _avg.reset();
}

/**
 * Symbol 1 = Mark (High)
 * Symbol 0 = Space (Low)
*/
void BaudotDecoder::processSample(uint8_t symbol) {

    _sampleCount++;

    // A windowed average is used to clean any noise out of the 
    // symbol transitions.
    int16_t rawSymbolQ15 = (symbol == 1) ? 32767 : -32767;
    int16_t smoothedSymbol = _avg.sample(rawSymbolQ15) >= 0 ? 1 : -1;

    // Watching for start bit
    if (_state == 0) {
        if (_lastSymbol == 1 && smoothedSymbol == -1) {
            _state = 1;
            _sampleCount = 0;
        }
    }
    else if (_state == 1) {
        if (_sampleCount == _samplesPerSymbol) {
            _state = 2;
            _sampleCount = 0;
            _symbolCount = 0;
            _symbolAcc = 0;
        }
    }
    else if (_state == 2) {
        // Wait for the middle of the symbol
        if (_sampleCount >= (_samplesPerSymbol >> 1)) {
            // Shift in a new symbol
            _symbolAcc <<= 1;
            if (smoothedSymbol == 1) {
                _symbolAcc |= 1;
            }
            _symbolCount++;
            // Wait out the rest of the data bit
            _state = 3;
        }
    }
    // This is the state where we wait through the data bit
    else if (_state == 3) {
        if (_sampleCount >= _samplesPerSymbol) {
            _sampleCount = 0;
            // Look for the completion of a character
            if (_symbolCount == 5) {
                // Look for shift codes
                if (_symbolAcc == BAUDOT_LTRS) {
                    _mode = BaudotMode::LTRS;
                } 
                // Look for shift codes
                else if (_symbolAcc == BAUDOT_FIGS) {
                    _mode = BaudotMode::FIGS;
                } 
                // Everything else is a normal character
                else {
                    // Convert from Baudot->ASCII
                    char asciiChar = 
                        BAUDOT_TO_ASCII_MAP[(_symbolAcc & 0b11111)]
                                        [(_mode == BaudotMode::LTRS) ? 0 : 1];
                    // Report the character
                    _listener->received(asciiChar);
                }
                // Start to wait for the stop bit to go by
                _state = 4;
            } else {
                // Go back to processing an incoming data bit
                _state = 2;
            }
        }
    }
    // This is the stop bit state
    else if (_state == 4) {
        if (_sampleCount >= _samplesPerSymbol) {
            _sampleCount = 0;
            _state = 0;
        }
    }

    _lastSymbol = smoothedSymbol;
}

}
