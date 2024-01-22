#include "Parameters.h"

namespace radlib {

static constexpr uint8_t MASKS[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

PackingState::PackingState()
:   bitPtr(0),
    bytePtr(0) {
}

bool SubSegParameters::isEqualTo(const SubSegParameters& other) const {
    return Nc == other.Nc &&
        bc == other.bc &&
        Mc == other.Mc &&
        xmaxc == other.xmaxc &&
        xMc[0] == other.xMc[0] &&
        xMc[1] == other.xMc[1] &&
        xMc[2] == other.xMc[2] &&
        xMc[3] == other.xMc[3] &&
        xMc[4] == other.xMc[4] &&
        xMc[5] == other.xMc[5] &&
        xMc[6] == other.xMc[6] &&
        xMc[7] == other.xMc[7] &&
        xMc[8] == other.xMc[8] &&
        xMc[9] == other.xMc[9] &&
        xMc[10] == other.xMc[10] &&
        xMc[11] == other.xMc[11] &&
        xMc[12] == other.xMc[12];
}

/**
 * Packs the sub-segment parameters into the specified area.
 * Please see table 1.1 on page 11 for full information.
*/
void SubSegParameters::pack(uint8_t* packArea, PackingState* state) const {        
    Parameters::pack1(packArea, state, Nc, 7);
    Parameters::pack1(packArea, state, bc, 2);
    Parameters::pack1(packArea, state, Mc, 2);
    Parameters::pack1(packArea, state, xmaxc, 6);
    for (uint16_t i = 0; i < 13; i++) {
        Parameters::pack1(packArea, state, xMc[i], 3);
    }
}

bool Parameters::isEqualTo(const Parameters& other) const {
    return 
        LARc[0] == other.LARc[0] &&
        LARc[1] == other.LARc[1] &&
        LARc[2] == other.LARc[2] &&
        LARc[3] == other.LARc[3] &&
        LARc[4] == other.LARc[4] &&
        LARc[5] == other.LARc[5] &&
        LARc[6] == other.LARc[6] &&
        LARc[7] == other.LARc[7] &&
        subSegs[0].isEqualTo(other.subSegs[0]) &&
        subSegs[1].isEqualTo(other.subSegs[1]) &&
        subSegs[2].isEqualTo(other.subSegs[2]) &&
        subSegs[3].isEqualTo(other.subSegs[3]);
}

void Parameters::pack(uint8_t* packArea, PackingState* state) const {        
    pack1(packArea, state, LARc[0], 6);
    pack1(packArea, state, LARc[1], 6);
    pack1(packArea, state, LARc[2], 5);
    pack1(packArea, state, LARc[3], 5);
    pack1(packArea, state, LARc[4], 4);
    pack1(packArea, state, LARc[5], 4);
    pack1(packArea, state, LARc[6], 3);
    pack1(packArea, state, LARc[7], 3);        
    subSegs[0].pack(packArea, state);
    subSegs[1].pack(packArea, state);
    subSegs[2].pack(packArea, state);
    subSegs[3].pack(packArea, state);
}

void Parameters::pack1(uint8_t* packArea, PackingState* state, uint16_t parameter, 
    uint16_t bits) {
    uint8_t work = (uint8_t)(parameter & 0xff);
    for (uint16_t b = 0; b < bits; b++) {
        if (work & 1) {
            packArea[state->bytePtr] |= MASKS[state->bitPtr];
        } else {
            packArea[state->bytePtr] &= ~MASKS[state->bitPtr];
        }
        work >>= 1;
        state->bitPtr++;
        // Look for the wrap to the next byte
        if (state->bitPtr == 8) {
            state->bytePtr++;
            state->bitPtr = 0;
        }
    }
}

}
