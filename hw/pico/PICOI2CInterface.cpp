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
#include <iomanip>

// PICO SDK
#include "hardware/i2c.h"

#include "PICOI2CInterface.h"

using namespace std;

namespace radlib {

PICOI2CInterface::PICOI2CInterface(i2c_inst_t* hw, ostream& str) 
:   _hw(hw),
    _str(str) { 
}

PICOI2CInterface::~PICOI2CInterface() {
}

void PICOI2CInterface::write(uint8_t addr, uint8_t data) {
    //int rc = i2c_write_blocking(i2c0, addr, &data, 1, false);
    int rc = i2c_write_blocking(_hw, addr, &data, 1, false);
    if (rc != 1) {
        cout << "WRITE ERROR" << endl;
    }
    _cycleCount++;
}

void PICOI2CInterface::write(uint8_t addr, uint8_t* data, uint16_t len) {
    int rc = i2c_write_blocking(_hw, addr, data, len, false);
    if (rc != len) {
        cout << "WRITE ERROR" << endl;
    }
    _cycleCount++;
}

uint8_t PICOI2CInterface::read(uint8_t addr) {
    uint8_t data = 0;
    int rc = i2c_read_blocking(_hw, addr, &data, 1, false);
    if (rc != 1) {
        cout << "READ ERROR" << endl;
    }
    _cycleCount++;
    return data;
}

}
