#ifndef _dsp_util_h
#define _dsp_util_h

//#include <cstdint>
#include <functional>
#include <cinttypes>

#include "../util/fixed_math.h"

namespace radlib {

float pi();

/**
 * Adds on to the index and wraps back to zero if necessary.
*/
uint16_t incAndWrap(uint16_t index, uint16_t size);

float complex_corr(cq15* c0, cq15* c1, uint16_t len);

float corr_real_complex(const q15* c0, const cq15* c1, uint16_t len);

/**
 * A utility function for dealing with circular buffers.  Result
 * is (base + disp) % size.
*/
uint16_t wrapIndex(uint16_t base, uint16_t disp, uint16_t size);

void visitTone(const unsigned int len, uint16_t sample_freq_hz, uint16_t tone_freq_hz,
    float amplitude, uint16_t phaseDegrees, std::function<void(uint16_t idx, float y)> cb);

void make_real_tone(q15* output, const unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phaseDegrees = 0);

void make_real_tone_distorted(q15* output, const unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phaseDegrees, float dcOffset);

void addTone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees = 0);

void make_complex_tone(cq15* output, unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phaseDegrees = 0);

}

#endif
