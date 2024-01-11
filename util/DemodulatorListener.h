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
#ifndef _DemodulatorListener_h
#define _DemodulatorListener_h

#include <cstdint>
#include "../util/fixed_math.h"
#include "../util/DataListener.h"

namespace radlib {

/**
 * An abstract interface used to receive status/events from the demodulator
 */
class DemodulatorListener : public DataListener {
public:

    virtual void dataSyncAcquired() { }
    virtual void dataSyncLost() { }
    virtual void frequencyLocked(uint16_t markFreq, uint16_t spaceFreq) { }
    virtual void goodFrameReceived() { }
    virtual void badFrameReceived(uint32_t rawFrame) { }
    // NOTE: This comes from the DataListener interface!
    virtual void received(char asciiChar) { }
    virtual void bitTransitionDetected() { }
    virtual void isSymbolPresent(bool e) { }
    virtual void receivedBit(bool bit, uint16_t frameBitPos, int syncFrameCorr) { }

    /**
     * The key data used to identify symbols.  
     * NOTE: This is called on every sample so be ready for a lot of data.
     */
    virtual void sampleMetrics(q15 sample, uint8_t activeSymbol, float* symbolCorr, bool isAnySymbolPresent) { }

    /**
     * Called when the receiver discards a duplicate codeword
     */
    virtual void discardedDuplicate() { }
};

}

#endif

