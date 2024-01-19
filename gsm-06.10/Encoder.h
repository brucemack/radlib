/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#ifndef _Encoder_h
#define _Encoder_h

namespace radlib {

class Encoder {

public:

    static const uint16_t SEGMENT_N = 160;

    Encoder();
    void reset();
    void encode(const int16_t sop[], uint8_t out[]);

private:

    // State preserved between segments
    int16_t _z1;
    int32_t _L_z2;
    int16_t _mp;
    int16_t _LARpp_last[9];
    int16_t _u[8];
    // NOTE: Indexing in draft document is -120 to -1, but 
    // we treat this as 0 to 119.
    int16_t _dp[120];
};

}

#endif
