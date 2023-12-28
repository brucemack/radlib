/*
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

