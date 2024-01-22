#include <cstdint>
#include "wav_util.h"

using namespace std;

namespace radlib {

/**
 * Writes a 32-bit integer in little-endian format.
 */
static void write32LE(ostream& str, uint32_t i) {
    uint8_t b;
    b = i & 0xff;
    str.write((char*)&b, 1);
    b = (i >> 8) & 0xff;
    str.write((char*)&b, 1);
    b = (i >> 16) & 0xff;
    str.write((char*)&b, 1);
    b = (i >> 24) & 0xff;
    str.write((char*)&b, 1);
}

/**
 * Writes a 16-bit integer in little-endian format.
 */
static void write16LE(ostream& str, uint16_t i) {
    uint8_t b;
    b = i & 0xff;
    str.write((char*)&b, 1);
    b = (i >> 8) & 0xff;
    str.write((char*)&b, 1);
}

/**
 * Here is a decent reference: http://soundfile.sapp.org/doc/WaveFormat/
 */
void encodeFromPCM16(const int16_t pcm[], uint32_t samples, std::ostream& str, 
    uint16_t samplesPerSecond) {

    uint32_t len = (samples * 2) + 36;

    str.write("RIFF", 4);
    write32LE(str, len);
    str.write("WAVE", 4);

    uint16_t i16;
    uint32_t i32;

    str.write("fmt ", 4);
    // Sub Chunk 1 size
    i32 = 16;
    write32LE(str, i32);

    // PCM format
    i16 = 1;
    write16LE(str, i16);

    // Number of channels
    i16 = 1;
    write16LE(str, i16);

    // Sample rate
    i32 = samplesPerSecond;
    write32LE(str, i32);

    // Byte rate
    i32 = samplesPerSecond * 2;
    write32LE(str, i32);

    // Block align
    i16 = 2;
    write16LE(str, i16);

    // Bits per sample
    i16 = 16;
    write16LE(str, i16);

    str.write("data", 4);
    // Bytes of audio
    i32 = samples * 2;
    write32LE(str, i32);
    // The actual data 
    for (uint32_t i = 0; i < samples; i++) {
        write16LE(str, pcm[i]);
    }   
}

}

