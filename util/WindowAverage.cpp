#include "WindowAverage.h"

namespace radlib {

WindowAverage::WindowAverage(uint16_t windowSizeLog2, int16_t* windowArea) 
:   _windowSizeLog2(windowSizeLog2),
    _windowArea(windowArea) {
    reset();
}

void WindowAverage::reset() {
    _accumulator = 0;
    _windowPtr = 0;
    uint16_t areaSize = 1 << _windowSizeLog2;
    for (uint16_t i = 0; i < areaSize; i++) {
        _windowArea[i] = 0;
    }
}

int16_t WindowAverage::sample(int16_t s) {
    uint16_t ptrMask = (1 << _windowSizeLog2) - 1;
    _accumulator += s;
    _accumulator -= _windowArea[_windowPtr];
    _windowArea[_windowPtr] = s;
    // Increment and wrap
    _windowPtr = (_windowPtr + 1) & ptrMask;
    return _accumulator >> _windowSizeLog2;
}


}

