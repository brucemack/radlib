#include <iostream>
#include <cassert>

#include "../../util/fixed_math.h"
#include "../../util/fixed_fft.h"
#include "../../util/f32_fft.h"
#include "../../util/dsp_util.h"

using namespace std;
using namespace radlib;

static bool float_close(float a, float b) {
    return (std::abs(a - b) / b) < 0.01;
}

// Tests related to correlation
static void test_set_3() {

    const uint16_t sampleFreq = 2000;
    const float toneFreq = 500;

    // Correlate a real and complex signal.  See that the result is 0.5 * mag0 * mag1 when the 
    // two signals are perfectly matched, regardless of the phase.
    {
        const uint16_t filterToneN = 16;
        cf32 filterTone[filterToneN];

        // Make the filter tone
        make_complex_tone_cf32(filterTone, filterToneN,  sampleFreq, toneFreq, 1.0);

        // Make a real tone of data that we just received
        float buffer[filterToneN];
        make_real_tone_f32(buffer, filterToneN, sampleFreq, toneFreq, 1.0, 90.0);

        float corr0 = corr_f32_cf32(buffer, filterTone, filterToneN);
        cout << "Correlation 0:  " << corr0 << endl;
        assert(float_close(corr0, 0.5));
    }

    // Correlate a real and complex signal.  See that the result is 0.5 * mag0 * mag1 when the 
    // two signals are perfectly matched, regardless of the tone.
    {
        const uint16_t filterToneN = 16;
        cf32 filterTone[filterToneN];

        // Make the filter tone
        make_complex_tone_cf32(filterTone, filterToneN,  sampleFreq, toneFreq, 0.5);

        // Make a real tone of data that we just received
        float buffer[filterToneN];
        make_real_tone_f32(buffer, filterToneN, sampleFreq, toneFreq, 0.5, 90.0);

        float corr0 = corr_f32_cf32(buffer, filterTone, filterToneN);
        cout << "Correlation 1:  " << corr0 << endl;
        assert(float_close(corr0, 0.5 * 0.5 * 0.5));
    }

    // Correlate a real and complex signal using fixed point math.  See that the result is 
    // 0.5 * mag0 * mag1 when the two signals are perfectly matched, regardless of the tone.
    {
        const uint16_t filterToneN = 16;
        cq15 filterTone[filterToneN];

        // Make the filter tone
        make_complex_tone_cq15(filterTone, filterToneN,  sampleFreq, toneFreq, 0.5);

        // Make a real tone of data that we just received
        q15 buffer[filterToneN];
        make_real_tone_q15(buffer, filterToneN, sampleFreq, toneFreq, 0.5, 90.0);

        float corr0 = corr_q15_cq15(buffer, filterTone, filterToneN);
        cout << "Correlation 2:  " << corr0 << endl;
        assert(float_close(corr0, 0.5 * 0.5 * 0.5));
    }
}

static void test_set_2() {

    const uint16_t sampleFreq = 2000;
    const uint16_t fftN = 512;
    const float toneFreq = 500;

    // Filter coefficients for a low-pass filter.  Cut-off frequency 500 (-6dB),
    // transition bandwidth 200, Blackman window.
    // Generated using: https://fiiir.com/
    const float h_500[] = {
        0.000000000000000000,
        0.000000000000000000,
        0.000104510558271037,
        0.000000000000000000,
        -0.000495818851204150,
        0.000000000000000001,
        0.001371806503465499,
        -0.000000000000000002,
        -0.003052237966652560,
        0.000000000000000004,
        0.005996359574962824,
        -0.000000000000000006,
        -0.010847625757932812,
        0.000000000000000009,
        0.018581654022125804,
        -0.000000000000000012,
        -0.030992063476643916,
        0.000000000000000015,
        0.052465979242486326,
        -0.000000000000000017,
        -0.099016831949696085,
        0.000000000000000019,
        0.315883798406251526,
        0.500000939389133148,
        0.315883798406251581,
        0.000000000000000019,
        -0.099016831949696071,
        -0.000000000000000017,
        0.052465979242486339,
        0.000000000000000015,
        -0.030992063476643916,
        -0.000000000000000012,
        0.018581654022125808,
        0.000000000000000009,
        -0.010847625757932808,
        -0.000000000000000006,
        0.005996359574962827,
        0.000000000000000004,
        -0.003052237966652564,
        -0.000000000000000002,
        0.001371806503465498,
        0.000000000000000001,
        -0.000495818851204150,
        0.000000000000000000,
        0.000104510558271038,
        0.000000000000000000,
        0.000000000000000000
    };

    // Fixed point version of the same filter
    q15 h_500_q15[47];
    for (uint16_t i = 0; i < 47; i++) {
        h_500_q15[i] = f32_to_q15(h_500[i]);
    }

    // Make a simple tone
    const uint16_t samplesSize = 2000;
    float samples[samplesSize];
    make_real_tone_f32(samples, samplesSize, sampleFreq, toneFreq, 0.5);

    float unfilteredPower = 0;
    float filteredPower = 0;

    // FFT before any filtering
    {
        cf32 fftInput[fftN];
        cf32 fftResult[fftN];
        for (uint16_t i = 0; i < fftN; i++) {
            // NOTICE: We increase the sample amplitude by 2 to compensate for the window
            fftInput[i].r = samples[i];
            fftInput[i].i = 0;
        }
        simpleDFT(fftInput, fftResult, fftN);
        uint16_t maxBin = maxMagIdx(fftResult, 1, fftN / 2);
        float maxFreq = ((float)sampleFreq / (float)fftN) * (float)maxBin;
        assert(maxBin == 128);
        assert(float_close(fftResult[maxBin].mag(), 0.25));
        unfilteredPower = fftResult[maxBin].magSquared();

        cout << "Filter #1 (before)" << endl;
        cout << "  Bin    : " << maxBin << endl;
        cout << "  Freq   : " << maxFreq << endl;
        cout << "  Mag    : " << fftResult[maxBin].mag() << endl;
    }

    // FFT after filtering.  We expect to see the power cut in half
    {
        float filteredSamples[samplesSize];
        // Here is where we convolve the FIR filter coefficients with 
        // the tone.
        for (uint16_t i = 0; i < samplesSize; i++) {
            // Check to see whether we have enough samples to convolve yet
            if (i >= sizeof(h_500)) {
                float conv = 0;
                for (uint16_t k = 0; k < sizeof(h_500); k++) {
                    // Multiply-accumulate.  Looking backwards through the samples
                    // and forwards across the h[k] transfer function.
                    conv += samples[i - k] * h_500[k];
                }
                filteredSamples[i] = conv;
            } else {
                filteredSamples[i] = 0;
            }
        }

        cf32 fftInput[fftN];
        cf32 fftResult[fftN];
        for (uint16_t i = 0; i < fftN; i++) {
            // Notice: we are ignoring the zero section!
            fftInput[i].r = filteredSamples[i + sizeof(h_500)];
            fftInput[i].i = 0;
        }
        simpleDFT(fftInput, fftResult, fftN);
        uint16_t maxBin = maxMagIdx(fftResult, 1, fftN / 2);
        float maxFreq = ((float)sampleFreq / (float)fftN) * (float)maxBin;
        // Max tone in the same
        assert(maxBin == 128);
        assert(float_close(fftResult[maxBin].mag(), 0.25 / 2));
        filteredPower = fftResult[maxBin].magSquared();

        cout << "Filter #1 (after)" << endl;
        cout << "  Bin    : " << maxBin << endl;
        cout << "  Freq   : " << maxFreq << endl;
        cout << "  Mag    : " << fftResult[maxBin].mag() << endl;
    }

    // Compare the powers.  We are expecting to see -6dB given the filter design.
    assert(float_close(10.0 * std::log10(filteredPower / unfilteredPower), -6.0));

    // Demonstrate the application of the LPF using Q15 math
    {
        q15 filteredSamples[samplesSize];
        // Here is where we convolve the FIR filter coefficients with 
        // the tone.
        for (uint16_t i = 0; i < samplesSize; i++) {
            // Check to see whether we have enough samples to convolve yet
            if (i >= 47) {
                q15 conv = 0;
                for (uint16_t k = 0; k < 47; k++) {
                    // Multiply-accumulate.  Looking backwards through the samples
                    // and forwards across the h[k] transfer function.
                    q15 m = mult_q15(f32_to_q15(samples[i - k]), h_500_q15[k]);
                    conv += m;
                }
                filteredSamples[i] = conv;
            } else {
                filteredSamples[i] = 0;
            }
        }

        // FFT on the result to show that the power has been cut in half
        q15 fftTrigTable[fftN];
        cq15 fftResult[fftN];
        FixedFFT fft(fftN, fftTrigTable);

        // Do the FFT in the result buffer, including the window.  
        for (uint16_t i = 0; i < fftN; i++) {
            fftResult[i].r = filteredSamples[i];
            fftResult[i].i = 0;
        }
        fft.transform(fftResult);

        // Find max bin
        uint16_t maxBin = max_idx(fftResult, 1, fftN / 2);
        assert(maxBin == fftN / 4);

        cout << "Filter #1 (after) Q15 data" << endl;
        cout << "  Max Bin  : " << maxBin << endl;
        cout << "  Max Freq : " << fft.binToFreq(maxBin, sampleFreq) << endl;
        cout << "  Max Mag  : " << fftResult[maxBin].mag_f32() << endl;
    }
}

// This is a set of test that look at the FFT code.
//
void test_set_1() {

    const uint16_t sampleFreq = 2000;
    const uint16_t fftN = 512;
    const float toneFreq = 500;

    // Do a simple real DFT the old-fashioned way
    {
        cf32 fftInput[fftN];
        cf32 fftResult[fftN];
        float fftWindow[fftN];

        // Build the Hann window for the FFT (raised cosine)
        for (uint16_t i = 0; i < fftN; i++) {
            fftWindow[i] = (0.5 * (1.0 - std::cos(2.0 * pi() * ((float) i) / ((float)fftN))));
        }
        // Make a simple tone
        const uint16_t samplesSize = 2000;
        float samples[samplesSize];
        make_real_tone_f32(samples, samplesSize, sampleFreq, toneFreq, 0.5);
        for (uint16_t i = 0; i < fftN; i++) {
            // NOTICE: We increase the sample amplitude by 2 to compensate for the window
            fftInput[i].r = (2.0 * samples[i]) * fftWindow[i];
            fftInput[i].i = 0;
        }
        simpleDFT(fftInput, fftResult, fftN);
        uint16_t maxBin = maxMagIdx(fftResult, 1, fftN / 2);
        assert(maxBin == fftN / 4);
        // We expect the magnitude to be half (the other half of the power is in the 
        // negative frequency bin)
        assert(float_close(fftResult[maxBin].mag(), 0.5 / 2));

        cout << "Simple Way (Real)" << endl;
        cout << " Max Bin  : " << maxBin << endl;
        cout << " Max Freq : " << ((float)sampleFreq / (float)fftN) * (float)maxBin << endl;
        cout << " Max Mag  : " << fftResult[maxBin].mag() << endl;
    }

    // FFT with float
    {
        float fftTrigTable[fftN];
        float fftWindow[fftN];
        cf32 fftResult[fftN];
        F32FFT fft(fftN, fftTrigTable);

        // Build the Hann window for the FFT (raised cosine)
        for (uint16_t i = 0; i < fftN; i++) {
            fftWindow[i] = (0.5 * (1.0 - std::cos(2.0 * pi() * ((float) i) / ((float)fftN))));
        }

        // Make a simple tone
        const uint16_t samplesSize = 2000;
        float samples[samplesSize];
        make_real_tone_f32(samples, samplesSize, sampleFreq, toneFreq, 0.5);
        // Do the FFT in the result buffer, including the window.  
        for (uint16_t i = 0; i < fftN; i++) {
            // NOTICE: We increase the sample by 2 to compensate for the window
            fftResult[i].r = (2.0 * samples[i]) * fftWindow[i];
            fftResult[i].i = 0;
        }
        fft.transform(fftResult);

        // Find max bin
        uint16_t maxBin = maxMagIdx(fftResult, 1, fftN / 2);
        assert(maxBin == fftN / 4);
        assert(float_close(fftResult[maxBin].mag(), 0.5 / 2));

        // Convert back to freq
        cout << "Complex Way #1" << endl;
        cout << "  Max Bin  : " << maxBin << endl;
        cout << "  Max Freq : " << fft.binToFreq(maxBin, sampleFreq) << endl;
        cout << "  Max Mag  : " << fftResult[maxBin].mag() << endl;

    }

    // FFT using q15 fixed-point math
    {
        q15 fftTrigTable[fftN];
        //q15 fftWindow[fftN];
        cq15 fftResult[fftN];
        FixedFFT fft(fftN, fftTrigTable);

        // Build the Hann window for the FFT (raised cosine)
        //for (uint16_t i = 0; i < fftN; i++) {
        //    fftWindow[i] = f32_to_q15(0.5 * (1.0 - std::cos(2.0 * pi() * ((float) i) / ((float)fftN))));
        //}

        // Make a simple tone
        const uint16_t samplesSize = 2000;
        q15 samples[samplesSize];
        make_real_tone_q15(samples, samplesSize, sampleFreq, toneFreq, 0.5);

        // Do the FFT in the result buffer, including the window.  
        for (uint16_t i = 0; i < fftN; i++) {
            //fftResult[i].r = mult_q15(samples[i], fftWindow[i]);
            fftResult[i].r = samples[i];
            fftResult[i].i = 0;
        }

        fft.transform(fftResult);

        // Find max bin
        uint16_t maxBin = max_idx(fftResult, 1, fftN / 2);
        assert(maxBin == fftN / 4);
        assert(float_close(fftResult[maxBin].mag_f32(), 0.5 / 2));

        cout << "Complex Way #2" << endl;
        cout << "  Max Bin  : " << maxBin << endl;
        cout << "  Max Freq : " << fft.binToFreq(maxBin, sampleFreq) << endl;
        cout << "  Max Mag  : " << fftResult[maxBin].mag_f32() << endl;
    }
}

int main(int,const char**) {
    test_set_1();
    test_set_2();
    test_set_3();
}
