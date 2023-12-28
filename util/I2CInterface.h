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
#ifndef _I2CInterface_h
#define _I2CInterface_h

#include <cstdint>

namespace radlib {

class I2CInterface {
public:

    virtual void write(uint8_t addr, uint8_t data) = 0;
    virtual void write(uint8_t addr, uint8_t* data, uint16_t len) = 0;
    virtual uint8_t read(uint8_t addr) = 0;

};

}

#endif
