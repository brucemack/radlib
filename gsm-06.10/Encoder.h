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
    int16_t z1;
    int32_t L_z2;
    int16_t mp;

};

}

#endif
