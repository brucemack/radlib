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
    assert(float_close(-135.0, newPhase));
}

static void test_set_3() {
    
    const uint32_t N = 256;
    const float sample_freq = 64000;
    float trig_area[N];
    F32FFT fft(N, trig_area);
    unsigned int maxBin;

    // 2khZ below carrier
    unsigned int lsb_bucket = 8;
    // 3kHz above carrier
    unsigned int usb_bucket = 12;

    // Create a two-tone signal at high frequency 
    f32 sig1[N];
    make_real_tone_f32(sig1, N, sample_freq, 20000, 1.0);
    f32 sig2[N];
    make_real_tone_f32(sig2, N, sample_freq, 25000, 1.0);
    f32 sig3[N];
    add_f32(sig3, sig1, sig2, N);

    // Make the Quadrature VFO
    f32 sig4[N];
    make_real_tone_f32(sig4, N, sample_freq, 22000);
    f32 sig5[N];
    make_real_tone_f32(sig5, N, sample_freq, 22000, 1.0, 90.0);

    // Quadrature mix the two-tone signal
    f32 sigI[N];
    mult_f32(sigI, sig4, sig3, N);
    f32 sigQ[N];
    mult_f32(sigQ, sig5, sig3, N);

    // Take the Hilbert transform of the Q signal
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
    float sigQ2[N];
    convolve_f32(sigQ2, sigQ, N, h, HN);

    // Delay the I signal
    float sigI2[N];
    delay_f32(sigI2, sigI, N, HN / 2);

    // USB test (validate supression of LSB)
    {
        // Combine the two sides to acheive cancelation of the 
        // upper or lower side-band.
        // Subtraction cancels the LSB
        float sigSSB[N];
        sub_f32(sigSSB, sigI2, sigQ2, N);

        // Check the spectrum of the SSB signal
        cf32 sigSSBC[N];
        convert_f32_cf32(sigSSBC, sigSSB, N);
        fft.transform(sigSSBC);
        // Positive frequencies only
        maxBin = maxMagIdx(sigSSBC, 0, N / 2);
        assert(maxBin == usb_bucket);

        // Look at side-band cancelation
        float ratio = sigSSBC[lsb_bucket].mag() / sigSSBC[usb_bucket].mag();
        float ratioDb = 20.0 * std::log10(ratio);
        cout << "LSB supression dB " << ratioDb << endl;
    }

    // LSB test (validate supression of USB)
    {
        // Combine the two sides to acheive cancelation of the 
        // upper or lower side-band
        // Addition cancels the USB
        float sigSSB[N];
        add_f32(sigSSB, sigI2, sigQ2, N);

        // Check the spectrum of the SSB signal
        cf32 sigSSBC[N];
        convert_f32_cf32(sigSSBC, sigSSB, N);
        fft.transform(sigSSBC);
        // Positive frequencies only
        maxBin = maxMagIdx(sigSSBC, 0, N / 2);
        assert(maxBin == lsb_bucket);

        // Look at side-band cancelation
        float ratio = sigSSBC[usb_bucket].mag() / sigSSBC[lsb_bucket].mag();
        float ratioDb = 20.0 * std::log10(ratio);
        cout << "USB supression dB " << ratioDb << endl;
    }
}

static void test_set_4() {
    
    const uint32_t N = 256;
    const float sample_freq = 64000;
    // The carrier frequency
    const float tune_freq = 22000;
    // The LSB tone, below carrier
    const float tone_0 = -2000;
    // The USB tone, above carrier
    const float tone_1 = +3000;

    float trig_area[N];
    F32FFT fft(N, trig_area);
    unsigned int maxBin;

    // Build the two-tone test
    const unsigned int SN = 1024;

    f32 sig0[SN];
    make_real_tone_f32(sig0, SN, sample_freq, tune_freq + tone_0, 1.0);
    f32 sig1[SN];
    make_real_tone_f32(sig1, SN, sample_freq, tune_freq + tone_1, 1.0);
    f32 sigRF[SN];
    add_f32(sigRF, sig0, sig1, SN);

    // Make the Quadrature VFO
    f32 sigVFO0[SN];
    make_real_tone_f32(sigVFO0, SN, sample_freq, tune_freq, 1.0, 0);
    f32 sigVFO1[SN];
    make_real_tone_f32(sigVFO1, SN, sample_freq, tune_freq, 1.0, 90.0);

    // Quadrature mix the two-tone signal
    f32 sigI[SN];
    mult_f32(sigI, sigVFO0, sigRF, SN);
    f32 sigQ[SN];
    mult_f32(sigQ, sigVFO1, sigRF, SN);

    // Make the Hilbert transform impulse.
    const unsigned int HN = 31;
    const float h[HN] = {
0.004195635890348866, -1.2790256324988477e-15, 0.009282101548804558, -3.220409857465908e-16, 0.01883580699770617, -8.18901417658659e-16, 0.03440100801932521, -6.356643085811313e-16, 0.059551575569702433, -8.708587876669048e-16, 0.10303763641989427, -6.507176308640055e-16, 0.19683153562363995, -1.8755360872545065e-16, 0.6313536408821954, 0, -0.6313536408821954, 1.8755360872545065e-16, -0.19683153562363995, 6.507176308640055e-16, -0.10303763641989427, 8.708587876669048e-16, -0.059551575569702433, 6.356643085811313e-16, -0.03440100801932521, 8.18901417658659e-16, -0.01883580699770617, 3.220409857465908e-16, -0.009282101548804558, 1.2790256324988477e-15, -0.004195635890348866
    };
    /*
    const float h[HN] = {
        0.020501678803366043, 
        -8.936550362402313e-06, 
        0.02134400086199499, 
        -1.5250983261459488e-05, 
        0.0326520397486797, 
        -1.994669152567787e-05, 
        0.04875589796856797, 
        -5.669758982698511e-06, 
        0.07296281528723697, 
        -6.837180026556937e-06, 
        0.11398417398227445, 
        -2.125424521352453e-05, 
        0.204017329268537, 
        -4.5917661706658896e-06, 
        0.633845838579163, 
        0, 
        -0.633845838579163, 
        4.5917661706658896e-06, 
        -0.204017329268537, 
        2.125424521352453e-05, 
        -0.11398417398227445, 
        6.837180026556937e-06, 
        -0.07296281528723697, 
        5.669758982698511e-06, 
        -0.04875589796856797, 
        1.994669152567787e-05, 
        -0.0326520397486797, 
        1.5250983261459488e-05, 
        -0.02134400086199499, 
        8.936550362402313e-06, 
        -0.020501678803366043
    };
    */
    float delayLineI[HN];
    float delayLineQ[HN];

    for (unsigned int i = 0; i < HN; i++) {
        delayLineI[i] = 0;
        delayLineQ[i] = 0;
    }

    float out[N];

    for (unsigned int i = 0; i < N; i++) {
        out[i] = 0;
    }

    // ===== Radio Loop ===================================================

    for (unsigned int n = 0; n < SN; n++) {

        // Shift the delay lines to the left
        for (unsigned int i = 0; i < HN - 1; i++) {
            delayLineI[i] = delayLineI[i + 1];
            delayLineQ[i] = delayLineQ[i + 1];
        }
        // Load latest samples onto the end of the delay line
        delayLineI[HN - 1] = sigI[n];
        delayLineQ[HN - 1] = sigQ[n];

        // Shift the output buffer
        for (unsigned int i = 0; i < N - 1; i++)
            out[i] = out[i + 1];

        // Perform Hilbert transform on Q
        float rq = 0;
        // Convolution - there is a flip here
        for (unsigned int i = 0; i < HN; i++)
            rq += h[i] * delayLineQ[HN - i - 1];
        // Pull the I data off the delay line according to the group delay
        // See pg. 202 in Lyons 1st Ed
        float ri = delayLineI[((HN - 1) / 2)];

        // LSB
        //out[N - 1] = rq + ri;
        // USB
        out[N - 1] = rq - ri;
    }

    // Check the spectrum of the SSB signal
    cf32 sigSSBC[N];
    convert_f32_cf32(sigSSBC, out, N);
    fft.transform(sigSSBC);
    // Positive frequencies only, with limits to avoid seeing
    // aliases on the high end of the N/2 space.
    maxBin = maxMagIdx(sigSSBC, 0, N / 4);
    cout << "Max Freq " << (sample_freq / N) * maxBin << endl;
    // Find second loudest freq
    float nextMax = 0;
    unsigned int nextMaxBin = 0;
    // Limiting this to avoid seeing aliases
    for (unsigned int i = 0; i < N / 4; i++) {
        if (i != maxBin && sigSSBC[i].mag() > nextMax) {
            nextMax = sigSSBC[i].mag();
            nextMaxBin = i;
        }
    }
    cout << "Next Max Freq " << (sample_freq / N) * nextMaxBin << endl;
    float ratio = sigSSBC[nextMaxBin].mag() / sigSSBC[maxBin].mag();
    cout << "Next Max Supression " << 20 * std::log10(ratio) << endl;

    //for (int i = 0; i < N / 2; i++) {
    //    cout << i << " " << sigSSBC[i].mag() << endl;
    //}
}

int main(int, const char**) {
    test_set_1();
    test_set_2();
    //test_set_3();
    test_set_4();
}


