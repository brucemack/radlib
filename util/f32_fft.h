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

PRIOR ART
=========
This borrows heavily from code by Hunter Adams (vha3@cornell.edu), 
Tom Roberts, and Malcolm Slaney (malcolm@interval.com).
*/
#ifndef _f32_fft_h
#define _f32_fft_h

#include <cmath>

#include "dsp_util.h"

namespace radlib {

/**
 * A floating-point FFT implementation using the original Danielson-Lanczos 
 * algorithm.
 * 
 * The goal is to create a straight-forward implementation that can 
 * be built on a desktop or embedded platforms.
 * 
 * There is no internal memory allocation/deallocation.
 */
class F32FFT {
public:

    F32FFT(uint16_t n, float* trigTableSpace);

    /**
     * Performs the FFT in-place. Meaning: the input series is overwritten.
     */
    void transform(cf32 f[]) const;

    float binToFreq(uint16_t bin, float sampleFreq) const;

private: 

    const uint16_t N;
    const uint16_t _log2N = std::log2(N);
    const uint16_t _shiftAmount = 16 - _log2N;
    const float PI = std::atan(1.0) * 4.0;
    const float TWO_PI = PI * 2.0f;

    // Pre-built table for fast trig
    float* _cosTable;
};

}

#endif
