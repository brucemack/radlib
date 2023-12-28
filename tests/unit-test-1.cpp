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
#include <sstream>
#include <string>
#include <cassert>

#include "../util/DataListener.h"
#include "../rtty/BaudotEncoder.h"
#include "../rtty/BaudotDecoder.h"

#include "TestFSKModulator.h"
#include "TestFSKModulator2.h"

using namespace std;
using namespace radlib;

// This is 45.45 baud
uint32_t symbolUs = 22002;

// Test listener
class Listener : public DataListener {
public:

    void received(char asciiChar) {
        _str << asciiChar;
    }

    string get() {
        string r = _str.str();
        _str.clear();
        return r;
    }

private:

    ostringstream _str;
};

static void run(BaudotDecoder& dec, uint8_t symbol, uint16_t copies) {
    for (uint16_t i = 0; i < copies; i++) {
        dec.processSample(symbol);
    }
}

int main(int,const char**) {
    
    TestFSKModulator mod;
    transmitBaudot("CQCQ DE KC1FSZ", mod, symbolUs);
    cout << endl;

    {
        int16_t windowArea[4];
        Listener l;
        BaudotDecoder dec(2000, 4545, 2, windowArea);
        dec.setDataListener(&l);

        // Idle
        run(dec, 1, 100);

        // Start bit
        run(dec, 0, 44);
        // Send 01110
        run(dec, 0, 44);
        run(dec, 1, 44);
        run(dec, 1, 44);
        run(dec, 1, 44);
        run(dec, 0, 44);
        // Stop bit
        run(dec, 1, 66);

        // Start bit
        run(dec, 0, 44);
        // Send 10111
        run(dec, 1, 44);
        run(dec, 0, 44);
        run(dec, 1, 44);
        run(dec, 1, 44);
        run(dec, 1, 44);
        // Stop bit
        run(dec, 1, 66);

        assert(l.get() == "CQ");
    }

    // Encode a message at the full sample rate and then decode it.
    {
        uint16_t sampleRate = 2000;
        uint16_t baudRateTimes100 = 4545;
        uint8_t sampleData[8000];
        uint32_t sampleDataSize = 8000;
        const char* msg = "CQCQ DE KC1FSZ";

        TestFSKModulator2 mod2(sampleRate, baudRateTimes100, sampleData, sampleDataSize);
        transmitBaudot(msg, mod2, symbolUs);
        // Make sure we didn't overflow
        assert(mod2.getSamplesUsed() < sampleDataSize);

        const uint16_t windowAreaSizeLog2 = 3;
        int16_t windowArea[1 << windowAreaSizeLog2];
        Listener l;
        BaudotDecoder dec(sampleRate, baudRateTimes100, windowAreaSizeLog2, windowArea);
        dec.setDataListener(&l);

        // Play all of the samples into the decoder
        for (uint32_t i = 0; i < mod2.getSamplesUsed(); i++) {
            dec.processSample(sampleData[i]);
        }
        assert(l.get() == msg);
   }
}


