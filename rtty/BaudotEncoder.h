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

/**
 * Transmits a null-terminated ASCII message using Baudot encoding. The 
 * assumption is that the receicer starts in LTRS mode. The receiver 
 * will be left in LTRS mode at the end of the transmission.
 * 
 * @param msg The null-terminated message in ASCII text.
 * @param mod The FSK modulator used to generate the symbols.
 * @param symbolLengthUs The length of each symbol in microseconds. Standard 
 *   45.45 baud uses a symbol length of 22,002 microseconds.
*/
void transmitBaudot(const char* msg, FSKModulator& mod, uint32_t symbolLengthUs);

}

#endif
