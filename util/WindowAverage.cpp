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
#include <algorithm>    // std::max
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
    if (_windowArea != 0) {
        uint16_t areaSize = 1 << _windowSizeLog2;
        for (uint16_t i = 0; i < areaSize; i++) {
            _windowArea[i] = 0;
        }
    }
}

int16_t WindowAverage::sample(int16_t s) {
    if (_windowArea == 0) {
        return s;
    } else {
        uint16_t ptrMask = (1 << _windowSizeLog2) - 1;
        _accumulator += s;
        _accumulator -= _windowArea[_windowPtr];
        _windowArea[_windowPtr] = s;
        // Increment and wrap
        _windowPtr = (_windowPtr + 1) & ptrMask;
        // Do the average division
        return _accumulator >> _windowSizeLog2;
    }
}

int16_t WindowAverage::getAvg() const {
    return _accumulator >> _windowSizeLog2;
}

int16_t WindowAverage::getMin() const {
    int16_t min = 0x7fff;
    if (_windowArea) {
        uint16_t areaSize = 1 << _windowSizeLog2;
        for (uint16_t i = 0; i < areaSize; i++) {
            min = std::min(min, _windowArea[i]);
        }
    }
    return min;
}

int16_t WindowAverage::getMax() const {
    int16_t max = -32767;
    if (_windowArea) {
        uint16_t areaSize = 1 << _windowSizeLog2;
        for (uint16_t i = 0; i < areaSize; i++) {
            max = std::max(max, _windowArea[i]);
        }
    }
    return max;
}

}

