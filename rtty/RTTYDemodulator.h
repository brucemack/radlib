/*
Copyright (C) 2024 - Bruce MacKinnon KC1FSZ

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
#ifndef _RTTYDemodulator_h
#define _RTTYDemodulator_h

#include <cstdint>

#include "../util/fixed_math.h"
#include "../util/fixed_fft.h"
#include "../util/DemodulatorListener.h"
#include "../util/Demodulator.h"
#include "BaudotDecoder.h"

namespace radlib {

/**
 * This contains all of the RTTY demodulator logic.  This is a stateful
 * object that will be called once every sample interval with the latest
 * available sample.
 */
class RTTYDemodulator : public Demodulator {
public:

    RTTYDemodulator(uint16_t sampleFreq, uint16_t lowestFreq,
        uint16_t log2fftN,
        q15* fftTrigTableSpace, q15* fftWindowSpace, cq15* fftResultSpace, 
        q15* bufferSpace);

    virtual void setListener(DemodulatorListener* listener);

    virtual void reset();

    uint32_t getSampleCount() const { return _decoder.getSampleCount(); }
    uint32_t getInvalidSampleCount() const { return _decoder.getInvalidSampleCount(); }

protected:

    virtual void _processSymbol(bool isSymbolValid, uint8_t symbol);

private:

    static const uint16_t _windowSizeLog2 = 3;
    static const uint16_t _windowSize = 1 << _windowSizeLog2;
    int16_t _window[_windowSize];
    BaudotDecoder _decoder;
};

}

#endif

