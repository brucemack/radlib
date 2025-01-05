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
 *
 * From: http://ac4m.us/RTTY.html
 *
 * The lower RF frequency is known as the SPACE frequency and the upper 
 * RF frequency is known as the MARK frequency. The difference between 
 * the two is known as the SHIFT. For amateur radio, the SHIFT has been 
 * standardized at 170 Hz. It is customary to refer to the MARK 
 * frequency as the frequency you are operating on. For example, if you
 * say you are transmitting on 14080.00 kHz, that means your MARK 
 * frequency is 14080.00 kHz and your SPACE frequency is 170 Hz lower, 
 * or 14079.83 kHz. 
 */
class RTTYDemodulator : public Demodulator {
public:

    RTTYDemodulator(uint16_t sampleFreq, uint16_t lowestFreq, uint16_t log2fftN,
        q15* fftTrigTableSpace, q15* fftWindowSpace, cq15* fftResultSpace, 
        q15* bufferSpace);

    virtual void setListener(DemodulatorListener* listener);

    virtual void reset();

    uint32_t getSampleCount() const { return _decoder.getSampleCount(); }

    uint32_t getInvalidSampleCount() const { return _decoder.getInvalidSampleCount(); }

protected:

    virtual void _processSymbol(bool isSymbolValid, uint8_t symbol);

private:

    BaudotDecoder _decoder;
};

}

#endif

