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
#include "RTTYDemodulator.h"

namespace radlib {

RTTYDemodulator::RTTYDemodulator(uint16_t sampleFreq, uint16_t lowestFreq, uint16_t log2fftN,
    q15* fftTrigTable, q15* fftWindow,
    cq15* fftResultSpace, q15* bufferSpace) : 
    Demodulator(sampleFreq, lowestFreq, log2fftN, fftTrigTable, fftWindow, fftResultSpace, bufferSpace),
    _decoder(sampleFreq, 4545, _windowSizeLog2, _window) {
}

void RTTYDemodulator::setListener(DemodulatorListener* listener) {
    // Give the listener to the base class
    Demodulator::setListener(listener);
    // Connect the listener to the Baudot decoder as well to report out characters received
    _decoder.setDataListener(listener);
}

void RTTYDemodulator::reset() {
    // Tell the base modulator to reset
    Demodulator::reset();
    // Reset the Baudot decoder
    _decoder.reset();
}

void RTTYDemodulator::_processSymbol(uint8_t symbol) {    
    _decoder.processSample(symbol);
}

}

