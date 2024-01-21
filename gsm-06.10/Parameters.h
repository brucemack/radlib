/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#ifndef _Parameters_h
#define _Parameters_h

#include <cstdint>

namespace radlib {

struct SubSegParameters {

    uint16_t Nc;
    uint16_t bc;
    uint16_t Mc;
    uint16_t xmaxc;
    uint16_t xMc[13];

    bool isEqualTo(const SubSegParameters& other) const {
        return Nc == other.Nc &&
          bc == other.bc &&
          Mc == other.Mc &&
          xmaxc == other.xmaxc;
        //  xMc[0] == other.xMc[0] &&
        //  xMc[1] == other.xMc[1] &&
        //  xMc[2] == other.xMc[2] &&
        //  xMc[3] == other.xMc[3] &&
        //  xMc[4] == other.xMc[4] &&
        //  xMc[5] == other.xMc[5] &&
        //  xMc[6] == other.xMc[6] &&
        //  xMc[7] == other.xMc[7] &&
        //  xMc[8] == other.xMc[8] &&
        //  xMc[9] == other.xMc[9] &&
        //  xMc[10] == other.xMc[10] &&
        //  xMc[11] == other.xMc[11] &&
        //  xMc[12] == other.xMc[12];
    }
};

struct Parameters {
    
    uint16_t LARc[8];
    SubSegParameters subSegs[4];

    bool isEqualTo(const Parameters& other) const {
        return 
            LARc[0] == other.LARc[0] &&
            LARc[1] == other.LARc[1] &&
            LARc[2] == other.LARc[2] &&
            LARc[3] == other.LARc[3] &&
            LARc[4] == other.LARc[4] &&
            LARc[5] == other.LARc[5] &&
            LARc[6] == other.LARc[6] &&
            LARc[7] == other.LARc[7] &&
            subSegs[0].isEqualTo(other.subSegs[0]);
            //subSegs[1].isEqualTo(other.subSegs[1]);
            //subSegs[2].isEqualTo(other.subSegs[2]) &&
            //subSegs[3].isEqualTo(other.subSegs[3]);
    }
};

}    

#endif
