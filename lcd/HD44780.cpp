#include <iostream>
#include "HD44780.h"

using namespace std;

/*
From Datasheet: 

[Pg 22]
For 4-bit interface data, only four bus lines (DB4 to DB7) are used for 
transfer. Bus lines DB0 to DB3 are disabled. The data transfer between 
the HD44780U and the MPU is completed after the 4-bit data has been 
transferred twice. As for the order of data transfer, the four high 
order bits (for 8-bit operation, DB4 to DB7) are transferred before 
the four low order bits (for 8-bit operation, DB0 to DB3).

Notes On Busy Flag
------------------
[Pg 22]
The busy flag must be checked (one instruction) after the 4-bit data 
has been transferred twice. Two more 4-bit operations then transfer 
the busy flag and address counter data.

Figure 09 on page 22 seems to show that there is no need to check
the busy flag between the MSB and LSB transfers of the 4-bit pair.

The timing specification on page 52 defines tcycE as a minimum of 
500ns, which seems to be the fundeamental cycle time of the system.
And the minimum enable pulse width is 230ns. 
*/
namespace radlib {

HD44780::HD44780(bool isFourBit, uint8_t displayLines, bool fontMode) 
:   _isFourBit(isFourBit),
    _displayLines(displayLines),
    _fontMode(fontMode) {
}

void HD44780::init() {
    // 8-bit initialization sequence
    if (!_isFourBit) {
        // 1
        _waitUs(15000);
        // 2
        _writeIR8(0x30);
        // 3
        _waitUs(4100);
        // 4
        _writeIR8(0x30);
        // 5
        _waitUs(100);
        // 6
        _writeIR8(0x30);
        // 7
        _waitUs(100);
        // Now we setup the usual way
        uint8_t word = 0x30;
        if (_displayLines == 2) {
            word |= 0x08;
        }
        if (_fontMode) {
            word |= 0x04;
        }
        _writeIR(word);
    }
    // 4-bit initialization. Please remember to send the data on the
    // LSB side.
    else {
        // 1
        _waitUs(1500);
        // 2
        _writeIR8(0x03);
        // 3
        _waitUs(4100);
        // 4
        _writeIR8(0x03);
        // 5
        _waitUs(100);
        // 6
        _writeIR8(0x03);
        // 7
        _waitUs(100);
        // 8: Switch modes
        _writeIR8(0x02);
        // Now we setup the usual way.  We start to use full 8-bit words
        // here since the send will be split into two.
        uint8_t word = 0x20;
        if (_displayLines == 2) {
            word |= 0x08;
        }
        if (_fontMode) {
            word |= 0x04;
        }
        // This will be split into two writes!
        _waitUs(100);
        _writeIR(word);
    }
}

void HD44780::clearDisplay() {
    waitUntilNotBusy();
    _writeIR(0x01);
}

void HD44780::returnHome() {
    waitUntilNotBusy();
    _writeIR(0x02);
}

void HD44780::setEntryMode(bool increment, bool shift) {
    uint8_t word = 0x04;
    word |= increment ? 0x02 : 0x00;
    word |= shift ? 0x01 : 0x00;
    waitUntilNotBusy();
    _writeIR(word);
}

void HD44780::setDisplay(bool displayOn, bool cursorOn, bool blinkOn) {    
    uint8_t word = 0x08;
    word |= displayOn ? 0x04 : 0x00;
    word |= cursorOn ? 0x02 : 0x00;
    word |= blinkOn ? 0x01 : 0x00;
    waitUntilNotBusy();
    _writeIR(word);
}

void HD44780::setCGRAMAddr(uint8_t a) {
    waitUntilNotBusy();
    _writeIR(0x40 | (a & 0b00111111));
}

void HD44780::setDDRAMAddr(uint8_t a) {
    waitUntilNotBusy();
    _writeIR(0x80 | (a & 0b01111111));
}

void HD44780::write(uint8_t d) {
    // Removing ths since it doesn't appear that the busy flag
    // is relevent to data writes.
    // waitUntilNotBusy();
    _writeDR(d);
}

void HD44780::write(uint8_t* data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        write(data[i]);
    }
}

uint8_t HD44780::read() {
    return _readDR();
}

bool HD44780::isBusy() {
    return (_readIR() & 0x80) != 0;
}

// TODO: CONSIDER A TIMEOUT FEATURE
void HD44780::waitUntilNotBusy() {
    while (isBusy()) { 
        _busyCount++;
    }
}

void HD44780::_writeDR(uint8_t d) {
    if (_isFourBit) {
        uint8_t space[2];
        space[0] = (d & 0xf0) >> 4; 
        space[1] = (d & 0x0f);
        _writeDR8Multi(space, 2); 
        //_writeDR8((d & 0xf0) >> 4);
        //_writeDR8((d & 0x0f));
    } else {
        _writeDR8(d);
    }
}

void HD44780::_writeIR(uint8_t d) {
    if (_isFourBit) {
        _writeIR8((d & 0xf0) >> 4);
        _writeIR8((d & 0x0f));
    } else {
        _writeIR8(d);
    }
}

uint8_t HD44780::_readDR() {
    if (_isFourBit) {
        uint8_t result = (_readDR8() << 4) & 0xf0;
        result |= _readDR8() & 0x0f;
        return result;
    } else {
        return _readDR8();
    }
}

uint8_t HD44780::_readIR() {
    if (_isFourBit) {
        uint8_t result = _readIR8() << 4;
        result |= _readIR8() & 0x0f;
        return result;
    } else {
        return _readIR8();
    }
}

void HD44780::writeLinear(Format format, 
    const uint8_t* data, uint8_t len, uint8_t startPos) {
    uint8_t lastAddr = 0;
    // Cycle across the data provided
    for (uint8_t i = 0; i < len; i++) {
        // Compute the address for the next transder.
        const uint8_t addr = _linearPosToAddr(format, startPos + i);
        // Range error check
        if (addr > 0x67) {
            return;
        }
        // Check to see if the device is already pointing to 
        // the desired location.  This is an important optimization
        // since it allows us to take advantage of the built-in
        // increment capabilty.
        if (i == 0 || addr != lastAddr) {
            setDDRAMAddr(addr);
            lastAddr = addr;
        }
        // Write the data
        if (data[i] != 0)
            write(data[i]);
        else 
            write(' ');
        // Assume the address is incremented automatically
        lastAddr++;
    }
}

void HD44780::setCursorLinear(Format format, uint8_t pos) {
    uint8_t addr = _linearPosToAddr(format, pos);
    setDDRAMAddr(addr);
}

uint8_t HD44780::_rowToBaseAddr(Format format, uint8_t linearRow) {
    if (format == Format::FMT_20x4) {
        if (linearRow == 0) {
            return 0;
        } else if (linearRow == 1) {
            return 0x40;
        } else if (linearRow == 2) {
            return 0x14;
        } else if (linearRow == 3) {
            return 0x54;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

uint8_t HD44780::_linearPosToAddr(Format format, uint8_t pos) {
    if (format == Format::FMT_20x4) {
        const uint8_t cols = 20;
        const uint8_t linearRow = pos / cols;
        const uint8_t col = pos % cols;
        const uint8_t baseAddr = _rowToBaseAddr(format, linearRow);
        return baseAddr + col;
    } 
    else {
        // CONSIDER ERROR HANDLING METHOD
        return 0;
    }
}

void HD44780::setCursor(Format format, uint8_t row, uint8_t col) {
    setDDRAMAddr(_rowToBaseAddr(format, row) + col);
}

}
