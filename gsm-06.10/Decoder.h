/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#ifndef _Decoder_h
#define _Decoder_h

#include "Parameters.h"

namespace radlib {

class Decoder {
public:

    Decoder();
    void reset();
    void decode(const Parameters* in, int16_t* outputPcm);

private:

    int16_t _nrp;
    int16_t _drp[160];
    int16_t _LARpp_last[9];
    int16_t _v[9];
    int16_t _msr;
};

}

#endif
