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
#include "TestI2CInterface.h"

using namespace std;

namespace radlib {

TestI2CInterface::TestI2CInterface(std::ostream& str) 
:   _str(str) { 
}

static void formatLowData(uint8_t loData, ostream& str) {
    if (loData & 0b1000) {
        str << "BL";
    } else {
       str << "  ";
    }
    if (loData & 0b0010) {
        str << ",RD";
    } else {
       str << ",WR";
    }
    if (loData & 0b0001) {
        str << ",DAT";
    } else {
       str << ",CMD";
    }
    if (loData & 0b0100) {
        str << ",EN";
    }
}

void TestI2CInterface::write(uint8_t addr, uint8_t data) {
    _cycleCount++;
    ios_base::fmtflags flags(_str.flags());
    uint8_t hiData = (data & 0xf0) >> 4;
    uint8_t loData = (data & 0x0f);
    _str << "I2C WRITE ADDR ";
    _str << setfill('0') << setw(2) << hex << (int)addr;
    _str << " DATA ";
    _str << setfill('0') << setw(1) << hex 
        << (int)hiData << ":";
    formatLowData(loData, _str);
    _str << endl;
    _str.flags(flags);
}

void TestI2CInterface::write(uint8_t addr, uint8_t* data, uint16_t len) {
    _cycleCount++;
    ios_base::fmtflags flags(_str.flags());
    _str << "I2C WRITE ADDR " << setfill('0') << setw(2) << hex << (int)addr;
    _str << endl;
    _str << " DATA [";
    for (uint16_t i = 0; i < len; i++) {
        uint8_t hiData = (data[i] & 0xf0) >> 4;
        uint8_t loData = (data[i] & 0x0f);
        _str << setfill('0') << setw(1) << hex
            << (int)hiData << ":" << setw(1) << hex << (int)loData << " ";
    }
    _str << "]" << endl << dec;
    _str.flags(flags);
}

uint8_t TestI2CInterface::read(uint8_t addr) {
    ios_base::fmtflags flags(_str.flags());
    _cycleCount++;
    _str << "I2C READ  ADDR " << setfill('0') << setw(2) << hex << (int)addr;
    _str << endl << dec;
    _str.flags(flags);
    return 0;
}

}
