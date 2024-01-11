#include "dsp_util.h"

namespace radlib {

uint16_t incAndWrap(uint16_t i, uint16_t size) {
    uint16_t result = i + 1;
    if (result == size) {
        result = 0;
    }
    return result;
}

uint16_t wrapIndex(uint16_t base, uint16_t disp, uint16_t size) {
    uint16_t target = base + disp;
    while (target >= size) {
        target -= size;
    }
    return target;
}


static const float PI = std::atan(1.0) * 4.0;

float pi() {
    return PI;
}

void visit_real_tone(uint32_t len, float sample_freq_hz, float tone_freq_hz,
    float amplitude, float phase_degrees, std::function<void(unsigned int idx, float y)> cb) {

    float omega = 2.0f * pi() * (tone_freq_hz / sample_freq_hz);
    float phi = 2.0f * pi() * (phase_degrees / 360.0);

    for (uint32_t i = 0; i < len; i++) {
        float sig = std::cos(phi) * amplitude;
        cb(i, sig);
        phi += omega;
    }
}

void addTone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phase_degrees) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output](unsigned int ix, float x) {
        output[ix] += f32_to_q15(x);
    };
    visit_real_tone(len, sample_freq_hz, tone_freq_hz, amplitude, phase_degrees, cb);
}

void make_real_tone_q15(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phase_degrees) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output](unsigned int ix, float x) {
        output[ix] = f32_to_q15(x);
    };
    visit_real_tone(len, sample_freq_hz, tone_freq_hz, amplitude, phase_degrees, cb);
}

void make_real_tone_f32(float* output, const uint16_t len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phase_degrees) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output](unsigned int ix, float x) {
        output[ix] = x;
    };
    visit_real_tone(len, sample_freq_hz, tone_freq_hz, amplitude, phase_degrees, cb);
}

void make_real_tone_distorted(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees, float dcOffset) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output, dcOffset](unsigned int ix, float x) {
        output[ix] = f32_to_q15(x + dcOffset);
    };
    visit_real_tone(len, sample_freq_hz, tone_freq_hz, amplitude, phaseDegrees, cb);
}

void make_complex_tone_cq15(cq15* output, unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees) {

    float omega = 2.0f * pi() * (tone_freq_hz / sample_freq_hz);
    float phi = 2.0f * pi() * (phaseDegrees / 360.0);

    for (unsigned int i = 0; i < len; i++) {
        float sig_i = std::cos(phi) * amplitude;
        output[i].r = f32_to_q15(sig_i);
        float sig_q = std::sin(phi) * amplitude;
        output[i].i = f32_to_q15(sig_q);
        phi += omega;
    }
}

void make_complex_tone_cf32(cf32* output, uint16_t len, 
    float sample_freq_hz, float tone_freq_hz, 
    float amplitude, float phase_degrees) {

    float omega = 2.0f * pi() * (tone_freq_hz / sample_freq_hz);
    float phi = 2.0f * pi() * (phase_degrees / 360.0);

    for (unsigned int i = 0; i < len; i++) {
        float sig_i = std::cos(phi) * amplitude;
        output[i].r = sig_i;
        float sig_q = std::sin(phi) * amplitude;
        output[i].i = sig_q;
        phi += omega;
    }
}

float complex_corr(cq15* c0, cq15* c1, uint16_t len) {

    float result_r = 0;
    float result_i = 0;

    for (uint16_t i = 0; i < len; i++) {
        float a = q15_to_f32(c0[i].r);
        float b = q15_to_f32(c0[i].i);
        float c = q15_to_f32(c1[i].r);
        // Complex conjugate
        float d = -q15_to_f32(c1[i].i);
        // Use the method that minimizes multiplication
        float ac = a * c;
        float bd = b * d;
        float a_plus_b = a + b;
        float c_plus_d = c + d;
        float p0 = a_plus_b * c_plus_d;
        result_r += (ac - bd);
        result_i += (p0 - ac - bd);
    }

    // Scale based on size of data
    result_r /= (float)len;
    result_i /= (float)len;

    return std::sqrt(result_r * result_r + result_i * result_i);
}

// TODO: CLEAN UP EFFICIENCY
float corr_q15_cq15(const q15* c0, const cq15* c1, uint16_t len)  {

    float result_r = 0;
    float result_i = 0;

    for (uint16_t i = 0; i < len; i++) {
        float a = q15_to_f32(c0[i]);
        float b = 0;
        float c = q15_to_f32(c1[i].r);
        // Complex conjugate
        float d = -q15_to_f32(c1[i].i);
        // Use the method that minimizes multiplication
        float ac = a * c;
        float bd = b * d;
        float a_plus_b = a + b;
        float c_plus_d = c + d;
        float p0 = a_plus_b * c_plus_d;
        result_r += (ac - bd);
        result_i += (p0 - ac - bd);
    }

    // Scale based on size of data
    result_r /= (float)len;
    result_i /= (float)len;

    return std::sqrt(result_r * result_r + result_i * result_i);
}

float corr_f32_cf32(const float* c0, const cf32* c1, uint16_t len) {

    float result_r = 0;
    float result_i = 0;

    for (uint16_t i = 0; i < len; i++) {
        // Real value
        float a = c0[i];
        // Real value
        float c = c1[i].r;
        // Complex conjugate value
        float d = -c1[i].i;
        result_r += (a * c);
        result_i += (a * d);
    }

    // Scale based on size of data
    result_r /= (float)len;
    result_i /= (float)len;
    
    return std::sqrt(result_r * result_r + result_i * result_i);
}

// TODO: CLEAN THIS UP FOR EFFICIENCY
float corr_q15_cq15_2(const q15* c0, uint16_t c0Base, uint16_t c0Size,
    const cq15* c1, uint16_t c1Size) {

    float result_r = 0;
    float result_i = 0;

    for (uint16_t i = 0; i < c1Size; i++) {
        // Real value
        float a = q15_to_f32(c0[wrapIndex(c0Base, i, c0Size)]);
        // Real value
        float c = q15_to_f32(c1[i].r);
        // Complex conjugate value
        float d = -q15_to_f32(c1[i].i);
        result_r += (a * c);
        result_i += (a * d);
    }

    // TODO: Improve efficiency
    result_r /= (float)c1Size;
    result_i /= (float)c1Size;

    // TODO: REMOVE!
    return std::sqrt(result_r * result_r + result_i * result_i);

    // We are using an approximation of the square/square root magnitude
    // calculator here:
    /*
    float abs_result_r = std::abs(result_r);
    float abs_result_i = std::abs(result_i);
    return std::max(abs_result_r, abs_result_i) + 
           std::floor((abs_result_r + abs_result_i) / 2.0);
    */
}

/**
 * This is an out-of-the-book implementation to use for sanity checking.
*/
void simpleDFT(cf32* in, cf32* out, uint16_t n) {
    for (uint16_t k = 0; k < n; k++) { 
        float sumreal = 0;
        float sumimag = 0;
        for (uint16_t t = 0; t < n; t++) {  // For each input element
            float angle = 2.0 * pi() * (float)t * (float)k / (float)n;
            sumreal +=  in[t].r * std::cos(angle) + in[t].i * std::sin(angle);
            sumimag += -in[t].r * std::sin(angle) + in[t].i * std::cos(angle);
        }
        out[k].r = sumreal / (float)n;
        out[k].i = sumimag / (float)n;
    }
}

uint16_t maxMagIdx(const cf32* data, uint16_t start, uint16_t dataLen) {
    float maxMag = 0;
    uint16_t maxIdx = 0;
    for (uint16_t i = start; i < dataLen; i++) {
        float mag = data[i].mag();
        if (mag > maxMag) {
            maxMag = mag;
            maxIdx = i;
        }
    }
    return maxIdx;
}

}
