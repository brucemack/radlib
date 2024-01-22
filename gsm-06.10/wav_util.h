/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#ifndef _wavutil_h
#define _wavutil_h

#include <cstdint>
#include <iostream>

namespace radlib {

/**
 * Takes a list of PCM samples (16-bit) and creates a .WAV stream.
*/
void encodeFromPCM16(const int16_t pcm[], uint32_t samples, std::ostream& str, 
    uint16_t samplesPerSecond);

void decodeToPCM16(std::istream& str, uint16_t pcm[], uint32_t maxSamples);

}

#endif


