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
#ifndef _TestModem2_h
#define _TestModem2_h

#include <cstdint>
#include "../../util/FSKModulator.h"

namespace radlib {

/**
 * Modulates into a memory buffer.
 */
class TestModem2 : public FSKModulator {
public:

    TestModem2(float* samples, unsigned int samplesSize, unsigned int sampleRate,
        unsigned int markFreq, unsigned int spaceFreq, float amp, 
        float dcBias = 0, float noiseAmp = 0);
    virtual ~TestModem2() { }

    virtual void sendSilence(uint32_t us);
    virtual void sendMark(uint32_t us);
    virtual void sendSpace(uint32_t us);

    uint32_t getSamplesUsed() const { return _samplesUsed; }

private:
    
    float _getNoise();

    float* _samples;
    unsigned int _samplesSize;
    uint32_t _samplesUsed = 0;
    unsigned int _sampleRate;
    unsigned int _markFreq;
    unsigned int _spaceFreq;
    float _amp;
    float _dcBias;
    float _noiseAmp;
    float _phi = 0;

};

}

#endif
