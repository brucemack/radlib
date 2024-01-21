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
    for (uint16_t k = 0; k < 160; k++) {
        _drp[k] = 0;
    }
    for (uint16_t i = 1; i <= 8; i++) {
        _LARpp_last[i] = 0;
    }
    for (uint16_t i = 0; i <= 8; i++) {
        _v[i] = 0;
    }
    _msr = 0;
}

void Decoder::decode(const Parameters* input, int16_t* outputPcm) {

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

        // Encoder Section 5.2.16 - APCM inverse quantization
        int16_t xMp[13];
        temp1 = Encoder::FAC[mant];
        temp2 = sub(6, exp);
        temp3 = 1 << sub(temp2, 1);
        for (uint16_t i = 0; i <= 12; i++) {
            // This subtraction is used to restore the sign of xMc[i]
            temp = sub((input->subSegs[j].xMc[i] << 1), 7);
            temp = temp << 12;
            temp = mult_r(temp1, temp);
            temp = add(temp, temp3);
            xMp[i] = temp >> temp2;
        }

        // Encoder Section 5.2.17 RPE grid positioning
        // ep[] is the reconstructed long term residual
        int16_t erp[40];
        for (uint16_t k = 0; k <= 39; k++) {
            erp[IX(k, 0, 39)] = 0;
        }
        for (uint16_t i = 0; i <= 12; i++) {
            erp[IX(input->subSegs[j].Mc + (3 * i), 0, 39)] = xMp[i];
        }

        // Section 5.3.1 - Long-Term Synthesis Filtering
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

        // Computation of the reconstructed short term residual signal drp[0..39]
        for (uint16_t k = 0; k <= 39; k++) {
            // NOTE: Index for _drp[] is different from draft doc
            uint16_t drpp = mult_r(brp, _drp[IX((k - Nr) + 120, 0, 159)]);
            // NOTE: Index for _drp[] is different from draft doc
            _drp[IX(k + 120, 0, 159)] = add(erp[k], drpp);
        }

        // Update the reconstructed short-term residual signal drp[-1..-120]
        for (uint16_t k = 0; k <= 119; k++) {
            _drp[IX((-120 + k) + 120, 0, 159)] = _drp[IX((-80 + k) + 120, 0, 159)];
        }
    }

    // Section 5.3.3 - Computation of the decoded reflection coefficients
    // The goal is to reconstruct rrp[1..8] 

    int16_t rrp[4][9];

    int16_t LARpp[9];
    // Here we have four sets of coefficients for different zones
    // in the segment
    int16_t LARp[4];

    // Encoder Section 5.2.8 - Decoding of the coded Log-Area Ratios

    // Compute the LARpp[1..8] by reversing the LARc[] values
    for (uint16_t i = 1; i <= 8; i++) {
        // NOTE: The addition of MIC[i] is used to restore the sign of LARc[i]
        int16_t temp1 = add(input->LARc[IX(i, 1, 8)], Encoder::MIC[IX(i, 1, 8)]) << 10;
        // Remember that B is scaled down twice as much as A, so we need up 
        // shift left to get on equal terms
        int16_t temp2 = Encoder::B[IX(i, 1, 8)] << 1;
        // Back out B
        temp1 = sub(temp1, temp2);
        // Divide by A 
        temp1 = mult_r(Encoder::INVA[IX(i, 1, 8)], temp1);
        // What is this for?
        LARpp[IX(i, 1, 8)] = add(temp1, temp1);
        // IMPORTANT: IT APPEARS THAT WE ARE STILL AT HALF-SCALE of the original r[]
    }    

    for (uint16_t i = 1; i <= 8; i++) {

        // Encoder Section 5.2.9.1 - Interpolation of the LARpp[1..8] to get the LARp[1..8]

        // Zone 0
        int16_t temp = add((_LARpp_last[i] >> 2), (LARpp[i] >> 2));
        LARp[0] = add(temp, (_LARpp_last[i] >> 1));
        // Zone 1
        LARp[1] = add((_LARpp_last[i] >> 1), (LARpp[i] >> 1));
        // Zone 2
        temp = add((_LARpp_last[i] >> 2), (LARpp[i] >> 2));
        LARp[2] = add(temp, (LARpp[i] >> 1));
        // Zone 3
        LARp[3] = LARpp[i];

        // NUMERICAL NOTE: THE LARp[]s ARE AT 0.5 SCALE HERE

        // Encoder Section 5.2.9.2 - Computation of the rp[1..8] from the interpolated 
        // LARp[1..8].  This is the reverse of the simplified log-area 
        // quantization.

        for (uint16_t zone = 0; zone < 4; zone++) {
            // The sign will be added back later
            temp = s_abs(LARp[zone]);
            // 11059/32767 = 0.3375 = 0.675/2
            if (temp < 11059) {
                // NUMERICAL NOTE: The 0.5 scale is removed. 
                temp = temp << 1;
            } 
            // 20070/32767 = 0.6125 = 1.225/2
            else if (temp < 20070) {
                // NUMERICAL NOTE: The 0.5 scale is removed. 
                temp = add(temp, 11059);
            } 
            // 26112/32767 ~= 0.796875.  
            // (NOTE: The approximation appears to be off a bit)
            else {
                // NUMERICAL NOTE: The 0.5 scale is removed, and 
                // before the addition so the coefficient is already
                // in full scale.
                temp = add((temp >> 2), 26112);
            }

            // NUMERICAL NOTE: At this point we are at full scale.
            rrp[zone][i] = temp;

            // Re-apply the sign if needed
            if (LARp[zone] < 0) {
                rrp[zone][i] = sub(0, rrp[zone][i]);
            }
        }
    }
   
    // NUMERICAL NOTE: At this point rrp[] is at full scale

    // Once we've used the LARpp_last, copy the new values for next time
    for (uint16_t i = 1; i <= 8; i++) {
        _LARpp_last[i] = LARpp[i];
    }

    // Section 5.3.4 - Short term synthesis filtering section
    //
    // This procedure uses the drp[0..39] signal and produces the sr[0...159] 
    // which is the output of the short-term synthesis filter.

    // Initialization of the array wt[0..159]

    int16_t sr[160];
    int16_t wt[160];

    for (uint16_t k = 0; k <= 39; k++) {
        // NOTE: Index differs from draft
        wt[k] = _drp[IX(k + 120, 0, 159)];
    }
    for (uint16_t k = 0; k <= 39; k++) {
        // NOTE: Index differs from draft
        wt[40 + k] = _drp[IX(k + 120, 0, 159)];
    }
    for (uint16_t k = 0; k <= 39; k++) {
        // NOTE: Index differs from draft
        wt[80 + k] = _drp[IX(k + 120, 0, 159)];
    }
    for (uint16_t k = 0; k <= 39; k++) {
        // NOTE: Index differs from draft
        wt[120 + k] = _drp[IX(k + 120, 0, 159)];
    }

    for (uint16_t k = 0; k <= 159; k++) {
        int16_t zone = Encoder::k2zone(k);
        int16_t sri = wt[k];
        for (uint16_t i = 1; i <= 8; i++) {
            sri = sub(sri, mult_r(rrp[zone][9 - i], _v[8 - i]));
            _v[9 - i] = add(_v[8 - 1], mult_r(rrp[zone][9 - i], sri));
        }
        sr[k] = sri;
        _v[0] = sri;
    }

    // Section 5.3.5 - Deemphasis filtering
    int16_t sro[159];
    for (uint16_t k = 0; k <= 159; k++) {
        int16_t temp = add(sr[k], mult_r(_msr, 28180));
        _msr = temp;
        sro[k] = _msr;
    }

    // Section 5.3.6 - Upscaling of the output signal
    int16_t srop[159];
    for (uint16_t k = 0; k <= 159; k++) {
        // TODO: UNDERSTAND DIFFERENCE BETWEEN THIS AND SHIFT
        srop[k] = add(sro[k], sro[k]);
    }

    // Section 5.3.7 - Truncation of the output variable
    for (uint16_t k = 0; k <= 159; k++) {
        srop[k] = srop[k] >> 3;
        srop[k] = srop[k] << 3;
        outputPcm[k] = srop[k];
    }
}

}
