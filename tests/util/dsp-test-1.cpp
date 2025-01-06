#include <iostream>
#include <cassert>

#include "../../util/fixed_math.h"
#include "../../util/fixed_fft.h"
#include "../../util/f32_fft.h"
#include "../../util/dsp_util.h"

using namespace std;
using namespace radlib;

static bool float_close(float a, float b, float error = 0.01) {
    return (std::abs(a - b) / b) < error;
}

static const float PI = 3.1415626;

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
    convert_f32_cf32(sigC, sigR, N);

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
    
    const uint32_t N = 256;
    const float sample_freq = 64000;
    const float tone_freq = 2000;
    float trig_area[N];
    F32FFT fft(N, trig_area);
    unsigned int maxBin;

    // Make a real signal and then convert it to an complex (analytic)
    // signal.

    // Create the impulse response of a Hilbert transform
    const unsigned int HN = 16;
    float h[HN];
    float sumOfSquares = 0;
    for (unsigned int k = 0; k < HN; k++) {
        // n is centered at zero, per the classical definition of h(n)
        int n = k - HN / 2;
        // Special case of zero
        if (n == 0) 
            h[k] = 0.0;
        else
            h[k] = (sample_freq / (PI * n)) * (1.0 - cos(PI * n));
        sumOfSquares += h[k] * h[k];
    }
    float gain = sqrt(sumOfSquares);
    for (unsigned int k = 0; k < HN; k++) {
        h[k] /= gain;
    }

    // Make a real signal. NOTICE: 45 degrees of phase has been added.
    float sigI[N];
    make_real_tone_f32(sigI, N, sample_freq, tone_freq, 1.0, 45.0);
    cf32 sigIC[N];
    convert_f32_cf32(sigIC, sigI, N);
    fft.transform(sigIC);
    // Positive frequencies only
    maxBin = maxMagIdx(sigIC, 0, N / 2);
    assert(maxBin == 8);
    // Half of the power is in the positive bin
    assert(float_close(0.5, sigIC[maxBin].mag()));
    // Confirm the 45 degrees of phase
    float origPhase = 360.0 * (sigIC[maxBin].phase() / (2.0 * PI));
    assert(float_close(45.0, origPhase));

    // Delay and evalute phase
    f32 delayedSigI[N];
    // NOTE: We delay by the calculated group delay. Which is 
    // appoximately half of the taps on the Hilbert impulse: HN / 2.
    // It's not exactly half because we are dealing with an even
    // number of taps.
    delay_f32(delayedSigI, sigI, N, HN / 2);
    cf32 delayedSigIC[N];
    convert_f32_cf32(delayedSigIC, delayedSigI, N);
    fft.transform(delayedSigIC);
    // Positive frequencies only
    maxBin = maxMagIdx(delayedSigIC, 0, N / 2);
    // Nothing should change here
    assert(maxBin == 8);
    // About half of the power is in the positive bin.  
    // NOTE: We are testing using a slightly smaller power fraction here
    // because the delayed signal has a discontinuity in it (the blank
    // spot at the beginning) that cases some spectral leakage.
    assert(float_close(0.47, delayedSigIC[maxBin].mag()));
    // Confirm the phase. We should have the original +45 degrees and 
    // the -90 degrees introduced by the delay.
    float delayedPhase = 360.0 * (delayedSigIC[maxBin].phase() / (2.0 * PI));
    assert(float_close(-45.0, delayedPhase));

    // Convolve the original (undelayed) real signal with the Hilbert impulse.
    float sigQ[N];
    convolve_f32(sigQ, sigI, N, h, HN);
    // Show that the frequency hasn't changed, only the phase
    cf32 sigQC[N];
    convert_f32_cf32(sigQC, sigQ, N);
    fft.transform(sigQC);
    // Positive frequencies only
    maxBin = maxMagIdx(sigQC, 0, N / 2);
    assert(maxBin == 8);
    float newPhase = 360.0 * (sigQC[maxBin].phase() / (2.0 * PI));
    // The phase of the Hilbert-transformed signal has two factors:
    // 1. -90 that we expect from the Hilbert transform (the whole point)
    // 2. -90 that is caused by the group delay of the transformer itself.
    cout << "new phase " << newPhase << endl;
    assert(float_close(-135.0, newPhase));
}

int main(int, const char**) {
    test_set_1();
    test_set_2();
}


