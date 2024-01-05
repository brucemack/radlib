#ifndef _dsp_util_h
#define _dsp_util_h

#include <cstdint>

namespace radlib {

float pi();

/**
 * Adds on to the index and wraps back to zero if necessary.
*/
uint16_t incAndWrap(uint16_t index, uint16_t size);

/**
 * A utility function for dealing with circular buffers.  Result
 * is (base + disp) % size.
*/
uint16_t wrapIndex(uint16_t base, uint16_t disp, uint16_t size);

}

#endif
