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

#include "../../util/DataListener.h"
#include "../../rtty/BaudotEncoder.h"
#include "../../rtty/BaudotDecoder.h"
#include "../../rtty/RTTYDemodulator.h"

#include "../scamp/TestModem2.h"
#include "../scamp/TestDemodulatorListener.h"
#include "../TestFSKModulator.h"
#include "../TestFSKModulator2.h"

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
    
    {
        TestFSKModulator mod;
        transmitBaudot("CQCQ DE KC1FSZ", mod, symbolUs);
        cout << endl;

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

    {
        const uint16_t sampleFreq = 2000;
        const uint16_t lowFreq = 100;
        // The size of the FFT used for frequency acquisition
        const uint16_t log2fftN = 9;
        const uint16_t fftN = 1 << log2fftN;
        // Data area for modulated signal
        const unsigned int sampleSize = 2000 * 8;
        static float samples[sampleSize];
        // This is 45.45 baud
        uint32_t symbolUs = 22002;
        float markFreq = 2125;
        float spaceFreq = 2295;

        // This is the modem used for the demonstration.  Samples are written to a 
        // memory buffer. 
        TestModem2 modem2(samples, sampleSize, sampleFreq, markFreq, spaceFreq, 0.3, 0.0, 0.0);

        // This is a modem that is used to capture the data for printing.
        //int8_t printSamples[34 * 30];
        //TestModem printModem(printSamples, sizeof(printSamples), 1);

        const char* testMessage1 = "DE KC1FSZ, GOOD MORNING";
        transmitBaudot(testMessage1, modem2, symbolUs);
        // Make sure we didn't overflow
        assert(modem2.getSamplesUsed() < sampleSize);

        TestDemodulatorListener testListener(cout, 0, 2);

        // Space for the demodulator to work in (no dynamic memory allocation!)
        q15 trigTable[fftN];
        q15 window[fftN];
        q15 buffer[fftN];
        cq15 fftResult[fftN];

        RTTYDemodulator demod(sampleFreq, lowFreq, log2fftN,
            trigTable, window, fftResult, buffer);
        demod.setListener(&testListener);
        // Manually lock the spread receive frequencies
        // The spread is negative for RTTY
        float spread = markFreq - spaceFreq;
        cout << "Spread " << spread << endl;
        demod.setSymbolSpread(spread);
        demod.setFrequencyLock(markFreq);

        // Walk through the data one byte at a time.  We do something extra
        // each time we have processed a complete block.
        uint32_t samplePtr = 0;
        while (samplePtr < modem2.getSamplesUsed()) {            
            const q15 sample = f32_to_q15(samples[samplePtr++]);
            demod.processSample(sample);
        }

        cout << "LAST DC : " << demod.getLastDCPower() << endl;
        cout << "MARK HZ : " << demod.getMarkFreq() << endl;
        cout << "MESSAGE : " << testListener.getMessage() << endl;
    }
}

