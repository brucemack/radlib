#ifndef _TestHD44780_h
#define _TestHD44780_h

#include <iostream>
#include "../HD44780.h"

/**
 * A dummy implementation that prints data activity to a stream. Useful
 * for unit testing.
*/
class TestHD44780 : public HD44780 {
public:

    TestHD44780(bool isFourBit, uint8_t displayLines, bool fontMode,
        std::ostream& str);

protected:

    virtual void _writeIR8(uint8_t d);
    virtual void _writeDR8(uint8_t d);
    virtual uint8_t _readDR();
    virtual uint8_t _readIR();
    virtual void _waitMs(uint16_t ms) const;

private:

    std::ostream& _str;
};

#endif
