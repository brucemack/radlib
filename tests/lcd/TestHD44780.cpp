#include <iostream>
#include <iomanip>
#include "TestHD44780.h"

using namespace std;

TestHD44780::TestHD44780(bool isFourBit, uint8_t displayLines, bool fontMode,
    ostream& str)
:   HD44780(isFourBit, displayLines, fontMode),
    _str(str) {
}

void TestHD44780::_writeIR8(uint8_t d) {
    _str << "WRITE IR: " << setfill('0') << setw(2) << right << hex << (int)d << endl;
}

void TestHD44780::_writeDR8(uint8_t d) {
    _str << "WRITE DR: " << setfill('0') << setw(2) << right << hex << (int)d << endl;
}

uint8_t TestHD44780::_readDR() {
    return 0;
}

uint8_t TestHD44780::_readIR() {
    // IMPORTANT: Busy flag is clear
    return 0x00;
}

void TestHD44780::_waitMs(uint16_t ms) const {
    _str << "WAIT    : " << ms << endl;
}
