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
#include "HD44780_PCF8574.h"

#define BLBIT (0b1000)
#define ENBIT (0b0100)
#define RWBIT (0b0010)
#define RSBIT (0b0001)

namespace radlib {

/*
Wiring:

       +-------------------------+
       |        HD44780          |
       |                         |
       | BL EN RW RS D7 D6 D5 D4 |
          
       | D3 D2 D1 D0 D7 D6 D5 D4 |         
       |                         |
       |         PCF8574         |
       +-------------------------+       

NOTE: [Page 8]

The master can then send a STOP or ReSTART condition or continue sending data. 
The number of data bytes that can be sent successively is not limited and the 
previous data is overwritten every time a data byte has been sent and 
acknowledged.
*/
HD44780_PCF8574::HD44780_PCF8574(uint8_t displayLines, bool fontMode,
    uint8_t i2cAddr, I2CInterface* i2c, ClockInterface* clk) 
:   HD44780(true, displayLines, fontMode), 
    _i2cAddr(i2cAddr),
    _i2c(i2c),
    _clk(clk) {
}

void HD44780_PCF8574::_write8(bool rsBit, uint8_t d) {
    // Move the LCD data up to the high 4 bits of the I2C data
    // since this is how the I2C interface is wired to the HD44780.
    uint8_t w0 = d << 4;
    // Turn on the backlight if requested
    if (_backLight) {
        w0 |= BLBIT;
    } 
    // Turn on the RS bit
    if (rsBit) {
        w0 |= RSBIT;
    }
    // Write with EN=0.  This is establishing the data before
    // the enable clock is strobed
    _i2c->write(_i2cAddr, (w0));
    // Write with EN=1.  This is the enable strobe.
    _i2c->write(_i2cAddr, (w0 | ENBIT));
    // Per datasheet page 52, the 
    // minimum pulse width (PWeh) is 230ns, which seems way
    // faster than an I2C cycle.
    // Write with EN=0.  This is the trailing edge fo the enable
    // strobe.
    _i2c->write(_i2cAddr, (w0));
}

/**
 * At the moment we are only supporting a two byte write. We create
 * two separate 4-bit write cycles via a 6-byte sequence.
*/
void HD44780_PCF8574::_writeDR8Multi(const uint8_t* d, uint16_t dLen) {
    if (dLen != 2) {
        // TODO: ERROR/CRASH
        return;
    }
    uint8_t space[6];
    uint8_t blMask = RSBIT;
    if (_backLight) {
        blMask |= BLBIT;
    }
    space[0] = (d[0] << 4) | blMask; 
    space[1] = (d[0] << 4) | blMask | ENBIT;
    space[2] = (d[0] << 4) | blMask;
    space[3] = (d[1] << 4) | blMask; 
    space[4] = (d[1] << 4) | blMask | ENBIT;
    space[5] = (d[1] << 4) | blMask;
    
    _i2c->write(_i2cAddr, space, 6);
}

/*
4 bits of LCD data are returned in the LSB bits of this
funciton's return.

From data sheet [pg 7]:

Input HIGH: The master needs to write 1 to the register to set 
the port as an input mode if the device is not in the default 
power-on condition. The master reads the register to check the 
input status. If the external source pulls the port pin up to 
VDD or drives logic 1, then the master will read the value of 1.

Input LOW: The master needs to write 1 to the register to set 
the port to input mode if the device is not in the default 
power-on condition. The master reads the register to check the 
input status. If the external source pulls the port pin down to 
VSS or drives logic 0, which sinks the weak 100 A current source, 
then the master will read the value of 0.

*/
uint8_t HD44780_PCF8574::_read8(bool rsBit) {
    // Turn on the read bit and keep the 4 data bits high to 
    // allow reading.  Please see the PCF8574 data sheet for
    // more information on this requirement.
    uint8_t w0 = 0b11110000 | RWBIT;
    // Turn on the backlight if requested
    if (_backLight) {
        w0 |= BLBIT;
    } 
    // Turn on the RS bit
    if (rsBit) {
        w0 |= RSBIT;
    }
    // Write with EN=0.  This is establishing the data before
    // the enable clock is strobed
    _i2c->write(_i2cAddr, (w0));
    // Write with EN=1.  This is the enable strobe.
    _i2c->write(_i2cAddr, (w0 | ENBIT));
    // Now read what is coming out of the LCD
    uint8_t readW0 = _i2c->read(_i2cAddr);
    // Write with EN=0.  This is the trailing edge fo the enable
    // strobe.
    _i2c->write(_i2cAddr, (w0));

    return (readW0 >> 4) & 0x0f;
}    

void HD44780_PCF8574::_writeDR8(uint8_t d) {
    _write8(true, d);
}

void HD44780_PCF8574::_writeIR8(uint8_t d) {
    _write8(false, d);
}

uint8_t HD44780_PCF8574::_readDR8() {
    return _read8(true);
}

uint8_t HD44780_PCF8574::_readIR8() {
    return _read8(false);
}

void HD44780_PCF8574::_waitUs(uint16_t us) const {
    _clk->sleepUs(us);
}

}
