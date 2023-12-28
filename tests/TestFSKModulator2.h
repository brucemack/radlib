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
#ifndef _TestFSKModulator2_h
#define _TestFSKModulator2_h

#include <cstdint>
#include "../util/FSKModulator.h"

namespace radlib {

class TestFSKModulator2 : public FSKModulator {
public:

    TestFSKModulator2(uint16_t sampleRate,
        uint16_t baudRateTimes100, 
        uint8_t* sampleData, uint32_t sampleDataSize);
    virtual ~TestFSKModulator2() { }

    void reset() {
        _sampleDataPtr = 0;
    }

    uint32_t getSamplesUsed() const {
        return _sampleDataPtr;
    }

    virtual void sendSilence(uint32_t durationUs);
    virtual void sendMark(uint32_t durationUs);
    virtual void sendSpace(uint32_t durationUs);

private:

    const uint16_t _sampleRate;
    //const uint16_t _baudRateTime100;
    uint8_t* _sampleData;
    const uint32_t _sampleDataSize;
    uint32_t _sampleDataPtr;
};

}

#endif
