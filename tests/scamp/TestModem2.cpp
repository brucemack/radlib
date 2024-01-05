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
#include <cmath>
#include <iostream>
#include <random>

#include "TestModem2.h"

using namespace std;

namespace radlib {

// Used for random noise generation
static std::random_device rd{};
static std::mt19937 gen{ rd() }; 
static std::normal_distribution<float> d(0.0, 1.0);

TestModem2::TestModem2(float* samples, unsigned int samplesSize, 
    unsigned int sampleRate,
    unsigned int markFreq, unsigned int spaceFreq, float amp, 
    float dcBias, float noiseAmp) 
:   _samples(samples),
    _samplesSize(samplesSize),
    _sampleRate(sampleRate),
    _markFreq(markFreq),
    _spaceFreq(spaceFreq),
    _amp(amp),
    _dcBias(dcBias),
    _noiseAmp(noiseAmp) {
}

float TestModem2::_getNoise() {
    if (_noiseAmp == 0) {
        return 0.0;
    } else {
        return _noiseAmp * d(gen);
    }
}

void TestModem2::sendSilence(uint32_t us) {
    // Convert us to samples
    uint32_t samples = (_sampleRate * us) / 1000000;
    for (unsigned int i = 0; i < samples; i++) {
        if (_samplesUsed < _samplesSize) {
            _samples[_samplesUsed++] = _dcBias + _getNoise();
        }
    }
}

void TestModem2::sendMark(uint32_t us) {
    // Convert us to samples
    uint32_t samples = (_sampleRate * us) / 1000000;
    float omega = 2.0f * 3.1415926f * (float)_markFreq / (float)_sampleRate;
    for (unsigned int i = 0; i < samples; i++) {
        if (_samplesUsed < _samplesSize) {
            _samples[_samplesUsed++] = _amp * std::cos(_phi) + _dcBias + _getNoise();
            _phi += omega;
        }
    }
}

void TestModem2::sendSpace(uint32_t us) {
    // Convert us to samples
    uint32_t samples = (_sampleRate * us) / 1000000;
    float omega = 2.0f * 3.1415926f * (float)_spaceFreq / (float)_sampleRate;
    for (unsigned int i = 0; i < samples; i++) {
        if (_samplesUsed < _samplesSize) {
            _samples[_samplesUsed++] = _amp * std::cos(_phi) + _dcBias + _getNoise();
            _phi += omega;
        }
    }
}

}

