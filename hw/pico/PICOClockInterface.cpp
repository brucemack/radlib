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
#include <iostream>

// PICO SDK
#include "pico/stdlib.h"

#include "PICOClockInterface.h"

using namespace std;

namespace radlib {

void PICOClockInterface::sleepMs(uint16_t ms) const {
    sleep_ms(ms);    
}

void PICOClockInterface::sleepUs(uint16_t us) const {
    sleep_us(us);
}

}
