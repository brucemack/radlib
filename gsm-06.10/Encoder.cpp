
#include "common.h"
#include "Encoder.h"

namespace radlib {


Encoder::Encoder() {
    reset();
}

void Encoder::reset() {
    z1 = 0;
    L_z2 = 0;
    mp = 0;
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

    int16_t so[SEGMENT_N];
    int16_t z1 = 0;
    int16_t s1 = 0;
    int32_t L_s2;
    int16_t msp;
    int16_t lsp;
    int16_t temp;
    int16_t sof[SEGMENT_N];
    int16_t s[SEGMENT_N];
    int16_t smax = 0;
    int16_t scalauto = 0;
    int32_t L_ACF[9];
    int32_t L_temp;
    int16_t ACF[9];
    int16_t P[9];
    int16_t r[9];
    int16_t LAR[9];
    int16_t K[9];

    // Section 5.2.1 - Scaling of the input variable
    for (uint16_t k = 0; k < SEGMENT_N; k++) {
        // Shift away the 3 low-order (dont' care) bits
        so[k] = sop[k] >> 3;
        // Back in q15 format divided by two
        so[k] = so[k] << 2;
    }

    // Section 5.2.2 - Offset compensation
    for (uint16_t k = 0; k < SEGMENT_N; k++) {

        s1 = sub(so[k], z1);
        z1 = so[k];

        L_s2 = s1;
        L_s2 = L_s2 << 15;

        msp = L_z2 >> 15;
        lsp = L_sub(L_z2, (msp << 15));
        temp = mult_r(lsp, 32735);
        L_s2 = L_add(L_s2, temp);
        L_z2 = L_add(L_mult(msp, 32735) >> 1, L_s2);

        sof[k] = L_add(L_z2, 16384) >> 15;
    }

    // Section 5.2.3 - Pre-emphasis
    for (uint16_t k = 0; k < SEGMENT_N; k++) {
        s[k] = add(sof[k], mult_r(mp, -28180));
        mp = sof[k];
    }

    // Section 5.2.4 - Autocorrelation
    //
    // The goal is to compute the array L_ACF[k].  The signal s[i] shall be scaled in order 
    // to avoid an overflow situation.

    // Search for the maximum
    smax = 0;
    for (uint16_t k = 0; k < SEGMENT_N; k++) {
        temp = abs(s[k]);
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
        for (uint16_t k = 0; k < SEGMENT_N; k++) {
            s[k] = mult_r(s[k], temp);
        }
    }

    // Compute the L_ACF[..]
    for (uint16_t k = 0; k <= 8; k++) {
        L_ACF[k] = 0;
        for (uint16_t i = k; i < SEGMENT_N; i++) {
            L_temp = L_mult(s[i], s[i - k]);
            L_ACF[k] = L_add(L_ACF[k], L_temp);
        }
    }

    // Rescaling of the array s[0..159]
    if (scalauto > 0) {
        for (uint16_t k = 0; k < SEGMENT_N; k++) {
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
            if (P[0] < abs(P[1])) {
                for (uint16_t i = n; i <= 8; i++) {
                    r[i] = 0;
                }
                // Continue with 5.2.6
                break;
            }
            r[n] = div(abs(P[1]), P[0]);
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

        // 5.2.6 Transformation of reflection coefficients to log-area 
        // ratios

        // Computation of the LAR[1..8] from the r[1..8]
        for (uint16_t i = 1; i <= 8; i++) {
            temp = abs(r[i]);
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
    }


}

}

