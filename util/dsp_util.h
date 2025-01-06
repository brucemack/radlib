#ifndef _dsp_util_h
#define _dsp_util_h

#include <functional>
#include <cinttypes>

#include "../util/fixed_math.h"

typedef float f32;

namespace radlib {

float pi();

/**
 * Floating-point complex number
*/
struct cf32 {

    float r = 0;
    float i = 0;

    cf32() : r(0), i(0) {}
    cf32(float ar, float ai) : r(ar), i(ai) {}
    cf32(const cf32& other) : r(other.r), i(other.i) { }

    float mag() const {
        return std::sqrt(magSquared());
    }
    float magSquared() const {
        return r * r + i * i;
    }
    /**
     * @returns Phase angle in radians.
     */
    float phase() const {
        return std::atan2(i, r);
    }
    /**
     * @returns this number plus b.
     */
    cf32 add(cf32 b) const {
        return cf32(r + b.r, i + b.i);
    }
    /**
     * @returns this number times b.
     */
    cf32 mult(cf32 b) const {
        return cf32(r * b.r - i * b.i, r * b.i + i * b.r);
    }
};

void add_complex(cf32* p, const cf32* a, const cf32* b, unsigned int n);
void mult_complex(cf32* p, const cf32* a, const cf32* b, unsigned int n);

/**
* Populates the complex arrat with the values of the real array, setting the 
* imaginary part to 0. 
*/
void convert_f32_cf32(cf32* complexData, const float* realData, uint16_t n);

/**
 * Adds on to the index and wraps back to zero if necessary.
*/
uint16_t incAndWrap(uint16_t index, uint16_t size);

float complex_corr(cq15* c0, cq15* c1, uint16_t len);

float corr_q15_cq15(const q15* c0, const cq15* c1, uint16_t len);

float corr_f32_cf32(const float* c0, const cf32* c1, uint16_t len);

/**
 * This is a convolution function that supports a circular buffer
 * for the c0 (real) series and a linear buffer for the c1 (complex)
 * series. This would typically be used to convolve input samples
 * from an ADC with the complex coefficients of an FIR or quadrature
 * demodulator.
 * 
 * Automatic wrapping on the c0 buffer is used to avoid going off the end.
 */
float corr_q15_cq15_2(const q15* c0, uint16_t c0Start, uint16_t c0Size, 
    const cq15* c1, uint16_t c1Size);

/**
 * A utility function for dealing with circular buffers.  Result
 * is (base + disp) % size.
*/
uint16_t wrapIndex(uint16_t base, uint16_t disp, uint16_t size);

/**
 * Visits points on a real-valued sinusoidal tone.
 */
void visit_real_tone(uint32_t len, float sample_freq_hz, float tone_freq_hz,
    float amplitude, float phaseDegrees, std::function<void(uint16_t idx, float y)> cb);

/**
 * Fills a buffer with a real sinusoidal signal of the specified amplitude/frequency/
 * phase.
 */
void make_real_tone_f32(float* output, const uint16_t len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phase_degrees = 0);

/**
 * Fills a buffer with a real sinusoidal signal of the specified amplitude/frequency/
 * phase.
 */
void make_real_tone_q15(q15* output, const unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude = 1.0, float phase_degrees = 0);

void make_real_tone_distorted(q15* output, const unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude = 1.0, float phaseDegrees = 0, float dcOffset = 0);

void addTone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude = 1.0, float phaseDegrees = 0);

/**
 * Creates a complex (quadrature) tone.
 */
void make_complex_tone_cf32(cf32* output, uint16_t len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phaseDegrees = 0);

void make_complex_tone_cq15(cq15* output, unsigned int len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phaseDegrees = 0);

/**
 * An extremely simplistic DFT.  Not usable for performance-critical 
 * applications!
 */
void simpleDFT(cf32* in, cf32* out, uint16_t fftN);

/**
 * Searches across an array of complex numbers and finds the 
 * index with the largets magnitude.
 */
uint16_t maxMagIdx(const cf32* data, uint16_t start, uint16_t dataLen);

/**
 * NOTE: The series is pre-padded with zeros and the last samples are 
 * ignored. This may not be desirable.
 */
void convolve_f32(f32* sigQ, const f32* sigI, unsigned int n, const f32* h, 
    unsigned int hn);

/**
 * NOTE: The series is pre-padded with zeros and the last samples are 
 * ignored. This may not be desirable.
 */
void delay_f32(f32* out, const f32* in, unsigned int n, unsigned int delay);

}

#endif
