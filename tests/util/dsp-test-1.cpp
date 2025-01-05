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

// Some basic sanity checks of real and complex signals.
static void test_set_1() {
    
    const uint32_t N = 64;
    const float sample_freq = 64000;
    const float tone_freq = 2000;
    float trig_area[N];
    F32FFT fft(N, trig_area);

    // Make a real signal and validate the FFT
    float sigR[N];
    make_real_tone_f32(sigR, N, sample_freq, tone_freq, 1.0);
    cf32 sigC[N];
    convertf32cf32(N, sigR, sigC);

    // Check the DFT
    fft.transform(sigC);
    // Positive freq
    int maxBin = maxMagIdx(sigC, 0, N / 2);
    assert(maxBin == 2);
    // Half of the power is in the positive bin
    assert(float_close(0.5, sigC[maxBin].mag()));
    // Negative freq
    maxBin = maxMagIdx(sigC, N / 2, N);
    assert(maxBin == 62);
    // Half of the power is in the negative bin
    assert(float_close(0.5, sigC[maxBin].mag()));

    // Now create an analyic version of the same tone.  This 
    // demonstrate the key property of an analytic signal, specifically
    // that it has no negative frequency component.
    make_complex_tone_cf32(sigC, N, sample_freq, tone_freq, 1.0);
    // DFT should only result in the positive frequency
    fft.transform(sigC);
    // Positive freq
    maxBin = maxMagIdx(sigC, 0, N);
    assert(maxBin == 2);
    // All of the power is in the positive bin
    assert(float_close(1.0, sigC[maxBin].mag()));
    // Expect nothing in any other bins.
    for (unsigned int i = 0; i < N; i++) {
        if (i == 2) {
            continue;
        }
        assert(sigC[i].mag() < 0.0001);
    }

    // Generate a 1kHz tone and then up-convert it using another 1kHz tone.
    // Since this is all done using analyic signals we don't get any 
    // unwanted mixing images.
    {
        cf32 sig1C[N];
        make_complex_tone_cf32(sig1C, N, sample_freq, 1000, 1.0);
        cf32 sig2C[N];
        make_complex_tone_cf32(sig2C, N, sample_freq, 1000, 1.0);
        // Notice two things:
        // 1. Multiplication by a positive frequency performs an 
        //    up-conversion (i.e. result is higher)
        // 2. The result is one-sided!  There is no image created.
        cf32 sig3C[N];
        mult_complex(sig3C, sig1C, sig2C, N);
        // DFT should only result in the positive frequency
        fft.transform(sig3C);
        // Positive freq
        maxBin = maxMagIdx(sig3C, 0, N);
        assert(maxBin == 2);
        // All of the power is in the positive bin
        assert(float_close(1.0, sig3C[maxBin].mag()));
        for (unsigned int i = 0; i < N; i++) {
            if (i == 2) {
                continue;
            }
            assert(sig3C[i].mag() < 0.0001);
        }
    }

    // Generate a 8kHz tone and then down-convert it using a -6kHz tone.
    // Since this is all done using analyic signals we don't get any 
    // unwanted mixing images.
    {
        cf32 sig1C[N];
        make_complex_tone_cf32(sig1C, N, sample_freq, 8000, 1.0);
        cf32 sig2C[N];
        make_complex_tone_cf32(sig2C, N, sample_freq, -6000, 1.0);
        cf32 sig3C[N];
        mult_complex(sig3C, sig1C, sig2C, N);
        // DFT should only result in the positive frequency
        fft.transform(sig3C);
        // Positive freq
        maxBin = maxMagIdx(sig3C, 0, N);
        assert(maxBin == 2);
        // All of the power is in the positive bin
        assert(float_close(1.0, sig3C[maxBin].mag()));
        for (unsigned int i = 0; i < N; i++) {
            if (i == 2) {
                continue;
            }
            assert(sig3C[i].mag() < 0.0001);
        }
    }

    // Two tones (analytic).  Show that the DFTs are independent.
    {
        cf32 sig1C[N];
        make_complex_tone_cf32(sig1C, N, sample_freq, 2000, 1.0);
        cf32 sig2C[N];
        make_complex_tone_cf32(sig2C, N, sample_freq, 5000, 0.5);
        cf32 sig3C[N];
        add_complex(sig3C, sig1C, sig2C, N);

        // DFT should see both
        fft.transform(sig3C);
        // Positive freq
        maxBin = maxMagIdx(sig3C, 0, N);
        assert(maxBin == 2);
        // All of the power is in the positive bin
        // The signals don't impact each other
        assert(float_close(1.0, sig3C[maxBin].mag()));
    }

    // Two tone test (analytic).  Down-convert and see some negative components.
    {
        cf32 sig1C[N];
        make_complex_tone_cf32(sig1C, N, sample_freq, 2000, 1.0);
        cf32 sig2C[N];
        make_complex_tone_cf32(sig2C, N, sample_freq, 5000, 0.5);
        cf32 sig3C[N];
        add_complex(sig3C, sig1C, sig2C, N);

        // The down conversion carrier
        cf32 sig4C[N];
        make_complex_tone_cf32(sig4C, N, sample_freq, -3000, 1.0);

        // We expect 1.0 at -1000 Hz and 0.5 at +2000 Hz
        cf32 sig5C[N];
        mult_complex(sig5C, sig3C, sig4C, N);

        fft.transform(sig5C);
        // Positive freq
        maxBin = maxMagIdx(sig5C, 0, N);
        // This is the -1 bin
        assert(maxBin == 63);
        // All of the power is in the positive bin
        // The signals don't impact each other
        assert(float_close(1.0, sig5C[maxBin].mag()));
    }
}

static void test_set_2() {
    
    const uint32_t N = 64;
    const float sample_freq = 64000;
    const float tone_freq = 2000;
    float trig_area[N];
    F32FFT fft(N, trig_area);




}

int main(int, const char**) {
    test_set_1();
    test_set_2();
}


