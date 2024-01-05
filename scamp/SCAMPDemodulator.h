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
#ifndef _SCAMPDemodulator_h
#define _SCAMPDemodulator_h

#include <cstdint>

#include "../util/fixed_math.h"
#include "../util/fixed_fft.h"
#include "DemodulatorListener.h"
#include "ClockRecoveryPLL.h"
#include "ClockRecoveryDLL.h"
#include "Demodulator.h"

namespace radlib {

/**
 * This contains all of the SCAMP demodulator logic.  This is a stateful
 * object that will be called once every sample interval with the latest
 * available sample.
 */
class SCAMPDemodulator : public Demodulator {
public:

    SCAMPDemodulator(uint16_t sampleFreq, uint16_t lowestFreq,
        uint16_t log2fftN,
        q15* fftTrigTableSpace, q15* fftWindowSpace, cq15* fftResultSpace, 
        q15* bufferSpace);

    float getClockRecoveryPhaseError() const;

    uint16_t getFrameCount() const { return _frameCount; };

    int32_t getPLLIntegration() const {
        return 0;
    }

    virtual void reset();

protected:

    virtual void _processSymbol(uint8_t symbol);

private:

    ClockRecoveryDLL _dataClockRecovery;

    // Have we seen the synchronization frame?
    bool _inDataSync = false;
    // Here is where we accumulate data bits
    uint32_t _frameBitAccumulator = 0;
    // The number of bits received in the frame
    uint16_t _frameBitCount = 0;
    // The number of frames received
    uint16_t _frameCount = 0;
    // The last code word that was received (used for duplicate detection)
    uint16_t _lastCodeWord12 = 0;
};

}

#endif

