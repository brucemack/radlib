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
#ifndef _BaudotDecoder_h
#define _BaudotDecoder_h

#include <cstdint>
#include "../util/DataListener.h"

namespace radlib {

extern const uint8_t BAUDOT_LTRS;
extern const uint8_t BAUDOT_FIGS;
extern const char BAUDOT_TO_ASCII_MAP[32][2];

enum BaudotMode { LTRS, FIGS };

/*
From: https://www.aa5au.com/rtty/diddles-by-w7ay/

Even though Baudot is a 5 bit character code, 3 extra bits are added 
to provide character synchronization.  A start bit (space tone) is 
prepended to the Baudot code, and two stop bits (mark tone) are 
added after the Baudot code.  Thus the actual character is 8 bits.

In “rest” or idle condition, RTTY sends a continuous Mark tone.  After 
an idle (Mark) period, the Baudot stream decoder is going to wait 
for the first space tone to be demodulated.  The Baudot decoder then 
assumes this is a Start bit.  It then assumes that the tone 1/45.45 
seconds later represents the first bit of the Baudot character, the 
one 2/45.45 seconds later represents the second bit of the Baudot 
character, and so on.


http://ac4m.us/RTTY.html

The lower RF frequency is known as the SPACE frequency and the upper 
RF frequency is known as the MARK frequency. The difference between 
the two is known as the SHIFT. For amateur radio, the SHIFT has been 
standardized at 170 Hz. It is customary to refer to the MARK f
requency as the frequency you are operating on. For example, if you
 say you are transmitting on 14080.00 kHz, that means your MARK 
 frequency is 14080.00 kHz and your SPACE frequency is 170 Hz lower, 
 or 14079.83 kHz. 
*/

class BaudotDecoder {
public:

    BaudotDecoder(uint16_t sampleRate, uint16_t baudRateTimes100);

    void setDataListener(DataListener* l) { _listener = l; };

    void reset();

    /**
     * Symbol 1 = Mark (High)
     * Symbol 0 = Space (Low)
    */
    void processSymbol(uint8_t symbol);

private:    

    uint16_t _sampleRate;
    uint16_t _baudRateTimes100;
    BaudotMode _mode = BaudotMode::LTRS;

    DataListener* _listener;
};

}

#endif
