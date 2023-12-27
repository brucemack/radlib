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
#ifndef _BaudotEncoder_h
#define _BaudotEncoder_h

#include <cstdint>
#include "../util/FSKModulator.h"

namespace radlib {

void transmitBaudot(const char* data, FSKModulator& mod, uint32_t symbolLengthUs);

}

#endif
