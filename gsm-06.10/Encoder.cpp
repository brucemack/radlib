/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#include "common.h"
#include "Encoder.h"

namespace radlib {

// See table 5.1 on page 43
// NOTE: The 0th entry is not used.  The draft uses index [1..8]
static const int16_t A[9] = { 0, 20480, 20480, 20480, 20480, 13964, 15360, 8534, 9036 };
static const int16_t B[9] = { 0, 0, 0, 2048, -2560, 94, -1792, -341, -1144 };
static const int16_t MIC[9] = { 0, -32, -32, -16, -16, -8, -8, -4, -4 };
static const int16_t MAC[9] = { 0, 31, 31, 15, 15, 7, 7, 3, 3 };

// See table 5.2 on page 43
// NOTE: The 0th entry is not used.  The draft uses index [1..8]
static const int16_t INVA[9] = { 0, 13107, 13107, 13107, 13107, 19223, 17476, 31454, 29708 };

// Table 5.3a: Decision level of the LTP gain quantizer
// See page 43
static const int16_t DLB[4] = { 6554, 16384, 26214, 32767 };

// Table 5.3b: Quantization levels of the LTP gain quantizer
// See page 43
static const int16_t QLB[4] = { 3277, 11469, 21299, 32767 };

// Table 5.4: Coefficients of the weighting filter
// See page 43
static const int16_t H[11] = { -134, -374, 0, 2054, 5741, 8192, 5741, 2054, 0, -374, -134 };

// Table 5.5: Normalized inverse mantissa used to compute xM/xmax
// See page 44
static const int16_t NRFAC[8] = { 29128, 26215, 23832, 21846, 20165, 18725, 17476, 16384 };

// Table 5.6: Normalized direct mantissa used to compute xM/xmax
// See page 44
static const int16_t FAC[8] = { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };

/**
 * Converts an index k[0..159] to the zone[0..3] as defined in Table 3.2.
*/
static uint16_t k2zone(uint16_t k) {
    if (k <= 12) {
        return 0;
    } else if (k <= 26) {
        return 1;
    } else if (k <= 39) {
        return 2;
    } else { 
        return 3;
    }
}

Encoder::Encoder() {
    reset();
}

void Encoder::reset() {
    _z1 = 0;
    _L_z2 = 0;
    _mp = 0;
    for (uint16_t i = 1; i <= 8; i++) {
        _LARpp_last[i] = 0;
    }
    for (uint16_t i = 0; i < 8; i++) {
        _u[i] = 0;
    }
    for (uint16_t i = 0; i < 120; i++) {
        _dp[i] = 0;
    }
}

/**
 * Please see https://www.etsi.org/deliver/etsi_en/300900_300999/300961/06.01.00_40/en_300961v060100o.pdf 
 * for the source specification that was used for this CODEC.  References to page/section numbers
 * are taken from that specific version of the document (Draft ETSI EN 300 961 V6.1.0 (2000-07)).
 * 
 * Variable names from the draft are preserved, even when they violate common C++ coding
 * conventions.
*/
void Encoder::encode(const int16_t sop[], uint8_t out[]) {

    int16_t so[120];
    int16_t s1 = 0;
    int32_t L_s2;
    int16_t msp, lsp, exp, mant, xmax, xmaxc, itest;
    int16_t temp, temp1, temp2, temp3, di, sav, Nc, bc, bp, EM, Mc, R, S;
    int16_t sof[120];
    int16_t s[120];
    int16_t smax, dmax;
    int16_t scal, scalauto = 0;
    int32_t L_ACF[9];
    int32_t L_temp, L_max, L_result, L_power;
    int16_t ACF[9];
    int16_t P[9];
    int16_t r[9];
    int16_t LAR[9];
    int16_t K[9];
    int16_t LARc[9];
    int16_t LARpp[9];
    // Here we have four sets of coefficients for different zones
    // in the segment
    int16_t LARp[4];
    int16_t rp[4][9];
    // This is the short-term residual signal
    int16_t d[120];
    int16_t wt[50];
    int16_t x[40];
    int16_t dpp[40];
    // Long-term residual signal
    int16_t e[40];
    int16_t ep[40];
    int16_t xM[13];
    int16_t xMc[13];
    int16_t xMp[13];

    // Section 5.2.1 - Scaling of the input variable
    for (uint16_t k = 0; k < 120; k++) {
        // Shift away the 3 low-order (dont' care) bits
        so[k] = sop[k] >> 3;
        // Back in q15 format divided by two
        so[k] = so[k] << 2;
    }

    // Section 5.2.2 - Offset compensation
    for (uint16_t k = 0; k < 120; k++) {

        // Compute the non-recursive part
        s1 = sub(so[k], _z1);
        _z1 = so[k];

        // Compute the recursive part
        L_s2 = s1;
        L_s2 = L_s2 << 15;

        // Execution of a 31 by 16 bit multiplication
        msp = _L_z2 >> 15;
        lsp = L_sub(_L_z2, (msp << 15));
        temp = mult_r(lsp, 32735);
        L_s2 = L_add(L_s2, temp);
        _L_z2 = L_add(L_mult(msp, 32735) >> 1, L_s2);

        // Compute sof[k] with rounding
        sof[k] = L_add(_L_z2, 16384) >> 15;
    }

    // Section 5.2.3 - Pre-emphasis
    for (uint16_t k = 0; k < 120; k++) {
        s[k] = add(sof[k], mult_r(_mp, -28180));
        _mp = sof[k];
    }

    // Section 5.2.4 - Autocorrelation
    //
    // The goal is to compute the array L_ACF[k].  The signal s[i] shall be scaled in order 
    // to avoid an overflow situation.

    // Search for the maximum
    smax = 0;
    for (uint16_t k = 0; k < 120; k++) {
        temp = s_abs(s[k]);
        if (temp > smax) {
            smax = temp;
        }
    }

    // Computation of the scaling factor
    if (smax == 0) {
        scalauto = 0;
    } else {
        scalauto = sub(4, norm(smax << 16));
    }

    // Scaling of the array s[0..159]
    if (scalauto > 0) {
        temp = 16384 >> sub(scalauto, 1);
        for (uint16_t k = 0; k < 120; k++) {
            s[k] = mult_r(s[k], temp);
        }
    }

    // Compute the L_ACF[..]
    for (uint16_t k = 0; k <= 8; k++) {
        L_ACF[k] = 0;
        for (uint16_t i = k; i < 120; i++) {
            L_temp = L_mult(s[i], s[i - k]);
            L_ACF[k] = L_add(L_ACF[k], L_temp);
        }
    }

    // Rescaling of the array s[0..159]
    if (scalauto > 0) {
        for (uint16_t k = 0; k < 120; k++) {
            s[k] = s[k] << scalauto;
        }
    }

    // Section 5.2.5 Computation of the reflection coefficients

    // Schur recursion with 16 bit arithmetic

    if (L_ACF[0] == 0) {
        for (uint16_t i = 1; i <= 8; i++) {
            r[i] = 0;
        }
    } else {
        temp = norm(L_ACF[0]);
        for (uint16_t k = 0; k <= 8; k++) {
            ACF[k] = (L_ACF[k] << temp) >> 16;
        }

        // Initialize P[] and K[] for the recursion
        for (uint16_t i = 1; i <= 7; i++) {
            K[9-i] = ACF[i];
        }
        for (uint16_t i = 0; i <= 8; i++) {
            P[i] = ACF[i];
        }

        // Compute reflection coefficients
        for (uint16_t n = 1; n <= 8; n++) {
            if (P[0] < s_abs(P[1])) {
                for (uint16_t i = n; i <= 8; i++) {
                    r[i] = 0;
                }
                // Continue with 5.2.6
                break;
            }
            r[n] = div(s_abs(P[1]), P[0]);
            if (P[1] > 0) {
                r[n] = sub(0, r[n]);
            }
            if (n == 8) {
                break;
            }

            // Schur recursion
            P[0] = add(P[0], mult_r(P[1], r[n]));
            for (uint16_t m = 1; m <= 8 - n; m++) {
                P[m] = add(P[m + 1], mult_r(K[9-m], r[n]));
                K[9 - m] = add(K[9 - m], mult_r(P[m + 1], r[n]));
            }
        }
    }

    // 5.2.6 Transformation of reflection coefficients to log-area 
    // ratios

    // Computation of the LAR[1..8] from the r[1..8]
    for (uint16_t i = 1; i <= 8; i++) {
        temp = s_abs(r[i]);
        if (temp < 22118) {
            temp = temp >> 1;
        }
        else if (temp < 31130) {
            temp = sub(temp, 11059);
        } else {
            temp = sub(temp, 26112) << 2;
        }
        LAR[i] = temp;
        if (r[i] < 0) {
            LAR[i] = sub(0, LAR[i]);
        }
    }

    // Section 5.2.7. - Quantization and coding of the Log-Area Ratios

    // Computation for quantizing and coding the LAR[1..8]
    // A[], B[] are lookup tables 5.1
    for (uint16_t i = 1; i <= 8; i++) {
        temp = mult(A[i], LAR[i]);
        temp = add(temp, B[i]);
        temp = add(temp, 256);
        LARc[i] = temp >> 8;

        // Check if LARc[i] lies between MIN and MAX
        if (LARc[i] > MAC[i]) {
            LARc[i] = MAC[i];
        } else if (LARc[i] < MIC[i]) {
            LARc[i] = MIC[i];
        }
        // NOTE: The equation is used to make all of the LARc[i] positive
        LARc[i] = sub(LARc[i], MIC[i]);
    }

    // ===== SHORT TERM ANALYSIS FILTERING SECTION ===========================

    // Section 5.2.8 - Decoding of the coded Log-Area Ratios

    // Compute the LARpp[1..8]
    for (uint16_t i = 1; i <= 8; i++) {
        // NOTE: The addition of MIC[i] is used to restore the sign of LARc[i]
        temp1 = add(LARc[i], MIC[i]) << 10;
        temp2 = B[i] << 1;
        temp1 = sub(temp1, temp2);
        temp1 = mult_r(INVA[i], temp1);
        LARpp[i] = add(temp1, temp1);
    }    
    
    // NOTE: This section is tricky to follow int the documentation

    for (uint16_t i = 1; i <= 8; i++) {

        // Section 5.2.9.1 - Interpolation of the LARpp[1..8] to get the LARp[1..8]
        // Zone 0
        LARp[0] = add((_LARpp_last[i] >> 2), (LARpp[i] >> 2));
        LARp[0] = add(LARp[0], (_LARpp_last[i] >> 1));
        // Zone 1
        LARp[1] = add((_LARpp_last[i] >> 1), (LARpp[i] >> 1));
        // Zone 2
        LARp[2] = add((_LARpp_last[i] >> 2), (LARpp[i] >> 2));
        LARp[2] = add((LARpp[2]), (LARpp[i] >> 1));
        // Zone 3
        LARp[3] = LARpp[i];

        // Section 5.2.9.2 - Computation of the rp[1..8] from the interpolated 
        // LARp[1..8]

        for (uint16_t zone = 0; zone < 4; zone++) {
            temp = s_abs(LARp[zone]);
            if (temp < 11059) {
                temp = temp << 1;
            } else if (temp < 20070) {
                temp = add(temp, 11059);
            } else {
                temp = add((temp >> 2), 26112);
            }
            rp[zone][i] = temp;
            if (LARp[zone] < 0) {
                rp[zone][i] = sub(0, rp[zone][i]);
            }
        }
    }

    // Once we've used the LARpp_last, copy the new values for next time
    for (uint16_t i = 1; i <= 8; i++) {
        _LARpp_last[i] = LARpp[i];
    }

    // Section 5.2.10 - Short term analysis filtering

    // NOTE: The zone thing continues here since there are 4 different
    // sets of rp coefficients.

    // Calculate d[] which is the short-term residual
    for (uint16_t k = 0; k < 120; k++) {
        uint16_t zone = k2zone(k);
        di = s[k];
        sav = di;
        for (uint16_t i = 1; i <= 8; i++) {
            temp = add(_u[i - 1], mult_r(rp[zone][i], di));
            di = add(di, mult_r(rp[zone][i], _u[i - 1]));
            _u[i - 1] = sav;
            sav = temp;
        }
        d[k] = di;
    }

    // ===== LONG TERM PREDICTOR SECTION =====================================

    // Section 5.2.11 Calculation of the LTP parameters

    // Search of the optimum scaling of d[0..39]
    dmax = 0;
    for (uint16_t k = 0; k <= 39; k++) {
        temp = s_abs(d[k]);
        if (temp > dmax) {
            dmax = temp;
        }
    }

    temp = 0;
    if (dmax == 0) {
        scal = 0;
    } else {
        temp = norm(dmax << 16);
    }
    if (temp > 6) {
        scal = 0;
    } else {
        scal = sub(6, temp);
    }

    // Initialization of a working array wt[0..39]
    for (uint16_t k = 0; k <= 39; k++) {
        wt[k] = d[k] >> scal;
    }

    // Search for the maximum cross-correlation and coding of the LTP lag
    L_max = 0;
    // Index for max cross-corelation
    Nc = 40;    
    for (uint16_t lambda = 40; lambda <= 120; lambda++) {
        L_result = 0;
        for (uint16_t k = 0; k < 40; k++) {
            // NOTE: Index adjustment vs. draft doc
            L_temp = L_mult(wt[k], _dp[(k - lambda) + 120]);
            L_result = L_add(L_temp, L_result);
        }
        if (L_result > L_max) {
            Nc = lambda;
            L_max = L_result;
        }
    }

    // Rescaling of L_max
    L_max = L_max >> (sub(6, scal));

    // Initialization of a working array wt[0..39]
    for (uint16_t k = 0; k <= 39; k++) {
        // NOTE: Index adjustment vs. draft doc
        wt[k] = _dp[(k - Nc) + 120] >> 3;
    }

    // Compute the power of te reconstructed short term residual signal dp[..]
    L_power = 0;
    for (uint16_t k = 0; k <= 39; k++) {
        L_temp = L_mult(wt[k], wt[k]);
        L_power = L_add(L_temp, L_power);
    }

    // Normalization of L_max and L_power
    if (L_max <= 0) {
        bc = 0;
    } else if (L_max >= L_power) {
        bc = 3;
    } else {
        temp = norm(L_power);
        R = (L_max << temp) >> 16;
        S = (L_power << temp) >> 16;

        // Coding of the LTP gain
        if (R <= mult(S, DLB[0])) {
            bc = 0;
        } else if (R <= mult(S, DLB[1])) {
            bc = 1;
        } else if (R <= mult(S, DLB[2])) {
            bc = 2;
        } else {
            bc = 3;
        }
   }

   // Section 5.2.12 - Long term analysis filtering
   // In this part we have to decode the bc parameter to compute the samples
   // of the estimate of dpp[0..39].

   // Decoding of the coded LTP gain
   bp = QLB[bc];

    // Calculating the array e[0..39] and the array dpp[0..39]
    // e[] is the long-term residual signal
    for (uint16_t k = 0; k <= 39; k++) {
        // NOTE: Index adjustment vs. draft doc
        dpp[k] = mult_r(bp, _dp[(k - Nc) + 120]);
        e[k] = sub(d[k], dpp[k]);
    }

    // ===== RPE ENCODING SECTION =============================================

    // Section 5.2.13 - Weighting filter

    // Initialization of a temporary working array wt[0..49]
    for (uint16_t k = 0; k <= 4; k++) {
        wt[k] = 0;
    }    
    for (uint16_t k = 5; k <= 44; k++) {
        wt[k] = e[k-5];
    }
    for (uint16_t k = 45; k <= 49; k++) {
        wt[k] = 0;
    }

    // Compute the signal x[0..39]
    for (uint16_t k = 0; k <= 39; k++) {
        // Rouding of the output of the filter
        L_result = 8192;
        for (uint16_t i = 0; i <= 10; i++) {
            L_temp = L_mult(wt[k + i], H[i]);
            L_result = L_add(L_result, L_temp);
        }
        // Scaling x4
        L_result = L_add(L_result, L_result);
        L_result = L_add(L_result, L_result);
        x[k] = L_result >> 16;
    }

    // Section 5.2.14 - RPE grid selection
    EM = 0;
    Mc = 0;

    for (uint16_t m = 0; m <= 3; m++) {
        L_result = 0;
        for (uint16_t i = 0; i <= 12; i++) {
            temp1 = x[m + (3 * i)] >> 2;
            L_temp = L_mult(temp1, temp1);
            L_result = L_add(L_temp, L_result);
        }
        if (L_result > EM) {
            Mc = m;
            EM = L_result;
        }
    }

    // Down-sampling by a factor 3 to get teh selected xM[0..12] RPE 
    // sequence.
    for (uint16_t i = 0; i <= 12; i++) {
        xM[i] = x[Mc + (3 * i)];
    }

    // Section 5.2.15 - APCM quantization of the selected RPE sequence
    xmax = 0;
    for (uint16_t i = 0; i <= 12; i++) {
        temp = s_abs(xM[i]);
        if (temp > xmax) {
            xmax = temp;
        }
    }

    // Quantizing and coding of xmax to get xmaxc
    exp = 0;
    temp = xmax >> 9;
    itest = 0;
    for (uint16_t i = 0; i <= 5; i++) {
        if (temp <= 0) {
            itest = 1;
        }
        temp = temp >> 1;
        if (itest == 0) {
            exp = add(exp, i);
        }
    }
    temp = add(exp, 5);
    xmaxc = add((xmax >> temp), (exp << 3));

    // Quantizing and coding of the xM[0..12] RPE sequence to get xMc[0..12]

    // Compute exponent and mantissa of teh decoded version of xmaxc
    exp = 0;
    if (xmaxc > 15) {
        exp = sub((xmaxc >> 3), 1);    
    }
    mant = sub(xmaxc, (exp << 3));

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

    // Direct computation of xMc[0..12] using table 5.5
    // Normalized by the exponent
    temp1 = sub(6, exp);
    // See table 5.5 (inverse mantissa)
    temp2 = NRFAC[mant];
    for (uint16_t i = 0; i <= 12; i++) {
        temp = xM[i] << temp1;
        temp = mult(temp, temp2);
        // This equation is used to make all the xMc[i] positive
        xMc[i] = add((temp >> 12), 4);
    }

    // Section 5.2.16 - APCM inverse quantization
    temp1 = FAC[mant];
    temp2 = sub(6, exp);
    temp3 = 1 << sub(temp2, 1);
    for (uint16_t i = 0; i <= 12; i++) {
        // This subtraction is used to restore the sign of xMc[i]
        temp = sub((xMc[i] << 1), 7);
        temp = temp << 12;
        temp = mult_r(temp1, temp);
        temp = add(temp, temp3);
        xMp[i] = temp >> temp2;
    }

    // Section 5.2.17 RPE grid positioning
    // ep[] is the reconstructed long term residual
    for (uint16_t k = 0; k <= 39; k++) {
        ep[k] = 0;
    }
    for (uint16_t i = 0; i <= 12; i++) {
        ep[Mc + (3 * i)] = xMp[i];
    }

    // Section 5.2.18 - Update of the reconstructed short term residual
    // signal dp[-120,1].
    for (uint16_t k = 0; k <= 79; k++) {
        // Shift 80 items
        // Original document: 
        // First index: -120 -> -41
        // Second index: -80 -> -1
        //
        // NOTE: Here we have changed the notation from the draft document!
        _dp[(-120 + k) + 120] = _dp[(-80 + k) + 120];
    }
    for (uint16_t k = 0; k <= 39; k++) {
        // NOTE: Here we have changed the notation from the draft document!
        _dp[(-40 + k) + 120] = add(ep[k], dpp[k]);
    }
}

}
