/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#include <iostream>
#include <cassert>
#include "common.h"

#include "Encoder.h"
#include "Decoder.h"

// Utility
#define q15_to_f32(a) ((float)(a) / 32768.0f)

// Sanity checking function for index bounds
#define IX(x, lo, hi) (_checkIx(x, lo, hi))

namespace radlib {

// Sanity checking function for index bounds
static uint16_t _checkIx(uint16_t x, uint16_t lo, uint16_t hi) {
    assert(x >= lo && x <= hi);
    return x;
}

Decoder::Decoder() {
    reset();
}

void Decoder::reset() {
    _nrp = 40;
    for (uint16_t k = 0; k <= 159; k++) {
        _drp[k] = 0;
    }
    for (uint16_t i = 0; i <= 8; i++) {
        _LARpp_last[i] = 0;
    }
    for (uint16_t i = 0; i <= 8; i++) {
        _v[i] = 0;
    }
    _msr = 0;
}

void Decoder::decode(const Parameters* input, int16_t* outputPcm) {

    int16_t wt[160];

    // This part runs four times, once for each sub-segment.  In keeping with 
    // the draft convention, we use "j" to denote the sub-segment.

    for (uint16_t j = 0; j < 4; j++) {

        int16_t temp, temp1, temp2, temp3;

        // Section 5.3.1 - RPE Decoding 
        // The goal here is to reconstruct the long-term residual erp[0..39] signal
        // from the received parameters for this sub-segment (Mc, xmaxc, xMc[]).

        int16_t exp, mant, itest;

        // Compute exponent and mantissa of the decoded version of xmaxc
        exp = 0;
        mant = 0;

        if (input->subSegs[j].xmaxc > 15) {
            exp = sub((input->subSegs[j].xmaxc >> 3), 1);    
        }
        mant = sub(input->subSegs[j].xmaxc, (exp << 3));

        // Normalize mantissa0 <= mant <= 7
        if (mant == 0) {
            exp = -4;
            mant = 15;
        } else {
            itest = 0;
            for (uint16_t i = 0; i <= 2; i++) {
                if (mant > 7) {
                    itest = 1;
                }
                if (itest == 0) {
                    mant = add((mant << 1), 1);
                }
                if (itest == 0) {
                    exp = sub(exp, 1);
                }
            }
        }
        mant = sub(mant, 8);

        // ****** exp/mant compare works ********

        // Encoder Section 5.2.16 - APCM inverse quantization
        int16_t xMp[13];
        temp1 = Encoder::FAC[mant];
        temp2 = sub(6, exp);
        temp3 = 1 << sub(temp2, 1);
        for (int16_t i = 0; i <= 12; i++) {
            // This subtraction is used to restore the sign of xMc[i]
            temp = sub((input->subSegs[j].xMc[i] << 1), 7);
            temp = temp << 12;
            temp = mult_r(temp1, temp);
            temp = add(temp, temp3);
            xMp[i] = temp >> temp2;
        }

        // ****** xMp compare works *********

        // Encoder Section 5.2.17 RPE grid positioning
        // ep[] is the reconstructed long term residual
        int16_t erp[40];
        for (int16_t k = 0; k <= 39; k++) {
            erp[IX(k, 0, 39)] = 0;
        }
        for (int16_t i = 0; i <= 12; i++) {
            erp[IX(input->subSegs[j].Mc + (3 * i), 0, 39)] = xMp[i];
        }

        // ****** ep[] vs erp[] compare is good ******* 

        // Section 5.3.2 - Long-Term Synthesis Filtering
        // Use bc abd Nc to realize the long-term synthesis filtering

        int16_t Nr = input->subSegs[j].Nc;
        if (input->subSegs[j].Nc < 40) {
            Nr = _nrp;
        } else if (input->subSegs[j].Nc > 120) {
            Nr = _nrp;
        }
        _nrp = Nr;

        // Decoding of the LTP gain bc
        int16_t brp = Encoder::QLB[input->subSegs[j].bc];

        /* ******* brp matches bp ******* */

        // Computation of the reconstructed short term residual signal drp[0..39]
        for (int16_t k = 0; k <= 39; k++) {
            // NOTE: Index for _drp[] is different from draft doc
            int16_t drpp = mult_r(brp, _drp[IX((k - Nr) + 120, 0, 119)]);
            // NOTE: Index for _drp[] is different from draft doc
            _drp[IX((k + 120), 120, 159)] = add(erp[k], drpp);
        }

        // Update the reconstructed short-term residual signal drp[-1..-120]
        for (int16_t k = 0; k <= 119; k++) {
            _drp[IX((-120 + k) + 120, 0, 119)] = _drp[IX((-80 + k) + 120, 40, 159)];
        }

        // Load up the right part of the wt[] vector, based on which sub-segment
        // we are working on.
        for (int16_t k = 0; k <= 39; k++) {
            // NOTE: _drp[] index differs from draft
            wt[IX((j * 40) + k, 0, 159)] = _drp[IX(k + 120, 120, 159)];
        }
    }

    // Section 5.3.3 - Computation of the decoded reflection coefficients
    // The goal is to reconstruct rrp[1..8] 

    int16_t rrp[4][9];
    Encoder::decodeReflectionCoefficients(input, _LARpp_last, rrp);

    // NUMERICAL NOTE: At this point rrp[] is at full scale

    // Section 5.3.4 - Short term synthesis filtering section
    //
    // This procedure uses the drp[0..39] signal and produces the sr[0...159] 
    // which is the output of the short-term synthesis filter.

    int16_t sr[160];

    // See figure 3.5 on page 26 
    for (int16_t k = 0; k <= 159; k++) {
        int16_t zone = Encoder::k2zone(k);
        int16_t sri = wt[k];
        for (int16_t i = 1; i <= 8; i++) {
            sri = sub(sri, mult_r(rrp[zone][IX(9 - i, 1, 8)], _v[IX(8 - i, 0, 7)]));
            // Moving forward on _v[]
            _v[IX(9 - i, 1, 8)] = add(_v[IX(8 - i, 0, 7)], mult_r(rrp[zone][IX(9 - i, 1, 8)], sri));
        }
        sr[k] = sri;
        _v[0] = sri;
    }

    // TODO: COLLAPSE ALL OF THESE LOOPS

    // Section 5.3.5 - Deemphasis filtering
    int16_t sro[160];
    for (int16_t k = 0; k <= 159; k++) {
        // 28180/32767 = 0.86
        int16_t temp = add(sr[k], mult_r(_msr, 28180));
        _msr = temp;
        sro[k] = _msr;
    }

    // Section 5.3.6 - Upscaling of the output signal
    int16_t srop[160];
    for (int16_t k = 0; k <= 159; k++) {
        // TODO: UNDERSTAND DIFFERENCE BETWEEN THIS AND SHIFT
        srop[k] = add(sro[k], sro[k]);
        //outputPcm[k] = srop[k] & 0xfff8;
    }

    // Section 5.3.7 - Truncation of the output variable
    for (int16_t k = 0; k <= 159; k++) {
        srop[k] = srop[k] >> 3;
        srop[k] = srop[k] << 3;
        outputPcm[k] = srop[k];
    }
}

}
