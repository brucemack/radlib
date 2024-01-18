
#include "common.h"
#include "Encoder.h"

namespace radlib {


Encoder::Encoder() {
    reset();
}

void Encoder::reset() {
    z1 = 0;
    L_z2 = 0;
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
    }
}

}

