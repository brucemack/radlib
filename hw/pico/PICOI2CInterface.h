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
#ifndef _PICOI2CInterface_h
#define _PICOI2CInterface_h

#include <iostream>
#include "../../util/I2CInterface.h"

namespace radlib {

/**
 * Interface with PICO hardware
 */
class PICOI2CInterface : public I2CInterface {
public:

    PICOI2CInterface(i2c_inst_t* hw, std::ostream& str);
    virtual ~PICOI2CInterface();

    virtual void write(uint8_t addr, uint8_t data);
    virtual void write(uint8_t addr, uint8_t* data, uint16_t len);
    virtual uint8_t read(uint8_t addr);

    uint16_t getCycleCount() const { return _cycleCount; };

private:

    i2c_inst_t* _hw;
    std::ostream& _str;
    uint16_t _cycleCount = 0;
};

}

#endif
