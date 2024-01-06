/*
SCAMP Encoder/Decoder
Copyright (C) 2023 - Bruce MacKinnon KC1FSZ

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free 
Software Foundation, either version 3 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef _Util_h
#define _Util_h

#include <iostream>
#include <cinttypes>
#include <functional>

#include "Frame30.h"
#include "../util/fixed_math.h"
#include "../util/FSKModulator.h"

namespace radlib {

/**
 * Takes a string and returns it in char pairs.
*/
void makePairs(const char* in, std::function<void(char a, char b)> cb);

/**
 * @returns The number of frames written into the list
*/
unsigned int encodeString(const char* in, Frame30* outList, unsigned int outListSize, 
    bool includeSyncFrame);

void make_bar(std::ostream& str, unsigned int len);

void render_spectrum(std::ostream& str, const cq15* x, uint16_t fftN, uint16_t sampleFreq);

/**
 * Takes a null-terminated ASCII message, encodes it, and transmits
 * it using the modulator provided.
 */
uint16_t modulateMessage(const char* asciiMsg, FSKModulator& mod, uint32_t symbolUs,
    Frame30* frames, const uint16_t maxFrames);
}

#endif
