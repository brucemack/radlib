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
#include "TestFSKModulator2.h"

using namespace std;

namespace radlib {

TestFSKModulator2::TestFSKModulator2(uint16_t sampleRate,
    uint16_t baudRateTimes100, 
    uint8_t* sampleData, uint32_t sampleDataSize) 
:   _sampleRate(sampleRate),
    _sampleData(sampleData),
    _sampleDataSize(sampleDataSize),
    _sampleDataPtr(0) {
}

void TestFSKModulator2::sendSilence(uint32_t durationUs) {
    sendSpace(durationUs);
}

static uint8_t randomFlip(uint8_t desiredSymbol) {
    if (rand() % 100 > 92) {
        if (desiredSymbol == 1) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return desiredSymbol;
    }
}

void TestFSKModulator2::sendMark(uint32_t durationUs) {
    uint32_t samples = (_sampleRate * durationUs) / 1000000;
    for (uint32_t i = 0; i < samples; i++) {
        if (_sampleDataPtr < _sampleDataSize) {
            _sampleData[_sampleDataPtr++] = randomFlip(1);
        }
    }
}

void TestFSKModulator2::sendSpace(uint32_t durationUs) {
    uint32_t samples = (_sampleRate * durationUs) / 1000000;
    for (uint32_t i = 0; i < samples; i++) {
        if (_sampleDataPtr < _sampleDataSize) {
            _sampleData[_sampleDataPtr++] = randomFlip(0);
        }
    }
}

}
