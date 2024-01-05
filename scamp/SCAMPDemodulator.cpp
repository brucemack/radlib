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
#include <cstdint>
#include <iostream>
#include <cstring>

#include "../util/dsp_util.h"
#include "Util.h"
#include "SCAMPDemodulator.h"

using namespace std;

namespace radlib {

SCAMPDemodulator::SCAMPDemodulator(uint16_t sampleFreq, uint16_t lowestFreq, uint16_t log2fftN,
    q15* fftTrigTable, q15* fftWindow,
    cq15* fftResultSpace, q15* bufferSpace) : 
    Demodulator(sampleFreq, lowestFreq, log2fftN, fftTrigTable, fftWindow, fftResultSpace, bufferSpace),
    _dataClockRecovery(sampleFreq)
{
    // FSK SCAMP is 33.3 bits/second
    _dataClockRecovery.setClockFrequency(33);
    //_dataClockRecovery.setBitFrequencyHint(33);
}

void SCAMPDemodulator::reset() {

    Demodulator::reset();

    _inDataSync = false;
    _frameBitCount = 0;
    _lastCodeWord12 = 0;
    _dataClockRecovery.setLock(false);
}

float SCAMPDemodulator::getClockRecoveryPhaseError() const {
    return _dataClockRecovery.getLastPhaseError();
}

void SCAMPDemodulator::_processSymbol(uint8_t activeSymbol) {

    // Show the sample to the PLL for clock recovery
    bool capture = _dataClockRecovery.processSample(activeSymbol);

    // Process the sample if we are told to do so by the data clock
    // recovery PLL.
    if (capture) {

        // Bring in the next bit. 
        _frameBitAccumulator <<= 1;
        _frameBitAccumulator |= (activeSymbol == 1) ? 1 : 0;

        // Look for the synchronization frame by correlated with the magic sequence            
        const int syncFrameCorr = abs(
            Frame30::correlate30(_frameBitAccumulator, Frame30::SYNC_FRAME.getRaw())
        );

        _listener->receivedBit(activeSymbol == 1, _frameBitCount, syncFrameCorr);

        _frameBitCount++;

        // At all times are are looking for the sync frame, or something very close to it.
        if (syncFrameCorr > 28) {
            _inDataSync = true;
            _frameBitCount = 0;
            _frameCount++;
            _lastCodeWord12 = 0;
            _dataClockRecovery.setLock(true);
            _listener->dataSyncAcquired();
        }
        // Check to see if we have accumulated a complete data frame
        else if (_frameBitCount == 30) {

            _frameBitCount = 0;
            _frameCount++;

            if (_inDataSync) {
                Frame30 frame(_frameBitAccumulator & Frame30::MASK30LSB);

                _listener->goodFrameReceived();
                CodeWord24 cw24 = frame.toCodeWord24();
                CodeWord12 cw12 = cw24.toCodeWord12();

                if (!cw12.isValid()) {
                    _listener->badFrameReceived(frame.getRaw());
                } 
                else {
                    // Per SCAMP specification: "If the receiver decodes the same code multiple
                    // times before receiving a different code, it should discard the redundant
                    // decodes of the code word."
                    if (cw12.getRaw() == _lastCodeWord12) {                        
                        _listener->discardedDuplicate();
                    }
                    else {
                        Symbol6 sym0 = cw12.getSymbol0();
                        Symbol6 sym1 = cw12.getSymbol1();
                        if (sym0.getRaw() != 0) {
                            _listener->received(sym0.toAscii());
                        }
                        if (sym1.getRaw() != 0) {
                            _listener->received(sym1.toAscii());
                        }
                    }
                }

                _lastCodeWord12 = cw12.getRaw();
            }
        }
    }
}

}
