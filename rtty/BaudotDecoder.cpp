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

// This is a grid that maps Baudot characters to ASCII characters.  
// The first column is used in "letters" (LTRS) mode and the 
// second column is used in "figures" (FIGS) mode.

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

BaudotDecoder::BaudotDecoder(uint16_t sampleRate, uint16_t baudRateTimes100) 
:   _samplesPerSymbol((100 * sampleRate) / baudRateTimes100),
    _mode(BaudotMode::LTRS),
    // In the initial state we are waiting to see a mark that can be used
    // as the basis for the start bit.
    _state(5),
    _sampleCount(0),
    _totalSampleCount(0),
    _invalidSampleCount(0),
    _symbolCount(0),
    _symbolAcc(0) {
}

void BaudotDecoder::reset() {
    _mode = BaudotMode::LTRS;
    // In the initial state we are waiting to see a mark that can be used
    // as the basis for the start bit.
    _state = 5;
    _sampleCount = 0;
    _totalSampleCount = 0;
    _invalidSampleCount = 0;
    _symbolCount = 0;
    _symbolAcc = 0;
}

/**
 * Symbol 1 = Mark (High)
 * Symbol 0 = Space (Low)
*/
void BaudotDecoder::processSample(bool isSymbolValid, uint8_t symbol) {

    _sampleCount++;
    _totalSampleCount++;

    if (!isSymbolValid) {
        _invalidSampleCount++;
    }

    // Watching for start bit
    if (_state == 0) {
        // Trigger on the down transition with a valid symbol
        if (isSymbolValid && symbol == 0) {
            _state = 1;
            _sampleCount = 0;
            //cout << "Start at " << _totalSampleCount << " " << (float)_totalSampleCount / 330.0 << " " << _invalidSampleCount << endl;
        }
    }
    // Waiting through the start bit
    else if (_state == 1) {
        if (_sampleCount == _samplesPerSymbol) {
            _state = 2;
            _sampleCount = 0;
            _symbolCount = 0;
            _symbolAcc = 0;
        }
    }
    // Inside the beginning of a data bit
    else if (_state == 2) {
        // Wait for the middle of the symbol
        if (_sampleCount >= (_samplesPerSymbol >> 1)) {
            //cout << "  Capture " << smoothedSymbol << " " << avgSymbol << " " << " " << (float)_totalSampleCount / 330.0 << endl;
            // Shift in a new symbol
            _symbolAcc <<= 1;
            if (symbol == 1) {
                _symbolAcc |= 1;
            }
            _symbolCount++;
            // Wait out the rest of the data bit
            _state = 3;
        }
    }
    // This is the state where we wait through the second part of the data bit
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
    // This is the stop bit state.  Technically we want to wait 1.5 bits, but we
    // are only waiting 1.0 bit.
    else if (_state == 4) {
        if (_sampleCount >= _samplesPerSymbol) {
            _sampleCount = 0;
            _state = 5;
        }
    }
    // This is the state where we are waiting to see us go back to mark
    else if (_state == 5) {
        if (isSymbolValid && symbol == 1) {
            _sampleCount = 0;
            _state = 0;
        }
    }
}

}
