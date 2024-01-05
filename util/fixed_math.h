/*
SCAMP Encoder/Decoder
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
#ifndef _fixed_math_h
#define _fixed_math_h

#include <iostream>
#include <cstdint>
#include <cmath>

typedef int16_t q15;

// For multiplication we use a 32-bit result to avoid loss of the fraction
#define mult_q15(a,b) ( (q15)((((int32_t)(a)) * ((int32_t)(b))) >> 15) )
#define q15_to_f32(a) ((float)(a) / 32768.0f)
#define f32_to_q15(a) ((q15)((a) * 32768.0f)) 
#define int_to_q15(a) ((q15)(a << 15))
#define q15_to_int(a) ((int)(a >> 15))
#define char_to_q15(a) (q15)(((q15)(a)) << 15)
#define abs_q15(a) (std::abs(a))

namespace radlib {

struct cq15 {

    q15 r = 0;
    q15 i = 0;
    
    float mag_f32() const {
        float ref = q15_to_f32(r);
        float imf = q15_to_f32(i);
        return std::sqrt(ref * ref + imf * imf);
    }

    q15 approx_mag_q15() const {
        q15 abs_r = abs_q15(r);
        q15 abs_i = abs_q15(i);
        return std::max(abs_r, abs_i) + ((abs_r + abs_i) >> 1); 
    }

    float mag_f32_squared() const {
        float ref = q15_to_f32(r);
        float imf = q15_to_f32(i);
        return ref * ref + imf * imf;
    }

    void accumulate(cq15 c) {
        r += c.r;
        i += c.i;
    }

    static cq15 mult(cq15 c0, cq15 c1);
};

/**
 * Correlates the real part of two series.
*/
q15 corr_q15(q15* d0, q15* d1, uint16_t len);

/**
 * Returns the index with the maximum magnitude, starting at the 
 * specified location and.  NOTE: We only consider dataLen - start
 * samples in this check!
 * 
 * @param start The index to start checking at.
 * @param len The length of the data space.
 */
uint16_t max_idx(const cq15* data, uint16_t start, uint16_t dataLen);

/**
 * A faster version that uses fixed point and an approximation 
 * for the complex magnitude.
*/
uint16_t max_idx_2(const cq15* sample, uint16_t start, uint16_t len);

q15 max_q15(const q15* data, uint16_t dataLen);
q15 min_q15(const q15* data, uint16_t dataLen);
// NOTE: This will only work for data lengths that are a power of two!
q15 mean_q15(const q15* data, uint16_t log2DataLen);

/**
 * This is a convolution function that supports a circular buffer
 * for the c0 (real) series and a linear buffer for the c1 (complex)
 * series. This would typically be used to convolve input samples
 * from an ADC with the complex coefficients of an FIR or quadrature
 * demodulator.
 * 
 * Automatic wrapping on the c0 buffer is used to avoid going off the end.
 */
float corr_real_complex_2(const q15* c0, uint16_t c0Base, uint16_t c0Size, 
    const cq15* c1, uint16_t c1Len);

}

#endif
