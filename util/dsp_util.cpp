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


static const float PI = 3.14159265358979323846;

float pi() {
    return PI;
}

void visitTone(const unsigned int len, uint16_t sample_freq_hz, uint16_t tone_freq_hz,
    float amplitude, uint16_t phaseDegrees, std::function<void(unsigned int idx, float y)> cb) {

    float omega = 2.0f * pi() * ((float)tone_freq_hz / (float)sample_freq_hz);
    float phi = 2.0f * pi() * ((float)phaseDegrees / 360.0);

    for (unsigned int i = 0; i < len; i++) {
        float sig = std::cos(phi) * amplitude;
        cb(i, sig);
        phi += omega;
    }
}

void addTone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output](unsigned int ix, float x) {
        output[ix] += f32_to_q15(x);
    };
    visitTone(len, sample_freq_hz, tone_freq_hz, amplitude, phaseDegrees, cb);
}

void make_real_tone(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output](unsigned int ix, float x) {
        output[ix] = f32_to_q15(x);
    };
    visitTone(len, sample_freq_hz, tone_freq_hz, amplitude, phaseDegrees, cb);
}

void make_real_tone_distorted(q15* output, 
    const unsigned int len, float sample_freq_hz, 
    float tone_freq_hz, float amplitude, float phaseDegrees, float dcOffset) {
    // Make the callback for each point
    std::function<void(unsigned int, float)> cb = [output, dcOffset](unsigned int ix, float x) {
        output[ix] = f32_to_q15(x + dcOffset);
    };
    visitTone(len, sample_freq_hz, tone_freq_hz, amplitude, phaseDegrees, cb);
}

void make_complex_tone(cq15* output, 
    const unsigned int len, float sample_freq_hz, 
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
        result_r += (ac - bd) / (float)len;
        result_i += (p0 - ac - bd) / (float)len;
    }

    return std::sqrt(result_r * result_r + result_i * result_i);
}

// TODO: CLEAN UP EFFICIENCY
float corr_real_complex(const q15* c0, const cq15* c1, uint16_t len)  {

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
        result_r += (ac - bd) / (float)len;
        result_i += (p0 - ac - bd) / (float)len;
    }

    return std::sqrt(result_r * result_r + result_i * result_i);
}

}

