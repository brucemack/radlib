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
    uint8_t Nc;
    uint8_t bc;
    uint8_t Mc;
    uint8_t xmaxc;
    uint8_t xMc[13];
};

struct Parameters {
    
    uint8_t LARc[8];

    SubSegParameters subSegs[4];
};

}    

#endif

