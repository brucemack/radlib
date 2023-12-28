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
#ifndef _WindowAverage_h
#define _WindowAverage_h

#include <cstdint>

namespace radlib {

class WindowAverage {
public:

    /**
     * @param windowSizeLog2 The size of the window is expressed in log terms
     *   (i.e. number of bits).  So if the window has 8 entries then the 
     *   windowSizeLog2 should be 3.
    */
    WindowAverage(uint16_t windowSizeLog2, int16_t* windowArea);

    void reset();
    int16_t sample(int16_t s);

private:

    const uint16_t _windowSizeLog2;
    int16_t* _windowArea;
    int32_t _accumulator;
    uint16_t _windowPtr;
};

}


#endif
