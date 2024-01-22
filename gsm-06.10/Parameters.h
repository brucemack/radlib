/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#ifndef _Parameters_h
#define _Parameters_h

#include <cstdint>

namespace radlib {

class PackingState {
public:

    PackingState();

    uint16_t bitPtr;
    uint16_t bytePtr;

    uint16_t bitsUsed() const {
        return (bytePtr * 8) + bitPtr;
    }
};

/**
 * Sub-segment parameters.
 * 
 * NOTE: The parameters are represented by 16-bit integers for convenience
 * only - none of the parameters require that precision.
 */
class SubSegParameters {
public:

    uint16_t Nc;
    uint16_t bc;
    uint16_t Mc;
    uint16_t xmaxc;
    uint16_t xMc[13];

    bool isEqualTo(const SubSegParameters& other) const;

    /**
     * Packs the sub-segment parameters into the specified area.
     * Please see table 1.1 on page 11 for full information.
    */
    void pack(uint8_t* packArea, PackingState* state) const;
};

/**
 * Segment parameters.
 * 
 * NOTE: The parameters are represented by 16-bit integers for convenience
 * only - none of the parameters require that precision.
 */
class Parameters {
public:

    uint16_t LARc[8];
    SubSegParameters subSegs[4];

    bool isEqualTo(const Parameters& other) const;
    void pack(uint8_t* packArea, PackingState* state) const;

    static void pack1(uint8_t* packArea, PackingState* state, uint16_t parameter, 
        uint16_t bits);
};

}    

#endif
