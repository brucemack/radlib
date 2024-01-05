#include "dsp_util.h"

namespace radlib {

uint16_t incAndWrap(uint16_t i, uint16_t size) {
    uint16_t result = i + 1;
    if (result == size) {
        result = 0;
    }
    return result;
}

uint16_t wrapIndex(uint16_t base, uint16_t disp, uint16_t size) {
    uint16_t target = base + disp;
    while (target >= size) {
        target -= size;
    }
    return target;
}


static const float PI = 3.14159265358979323846;

float pi() {
    return PI;
}

}

