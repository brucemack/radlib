/*
SCAMP Encoder/Decoder
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
#include <fstream>
#include <cmath>
#include <sstream>
#include <cassert>
#include <cstring>

#include "../../scamp/Util.h"
#include "../../scamp/Symbol6.h"
#include "../../scamp/CodeWord24.h"
#include "../../scamp/Frame30.h"
#include "../../scamp/SCAMPDemodulator.h"

#include "../../util/fixed_math.h"
#include "../../util/FSKModulator.h"

#include "TestDemodulatorListener.h"
#include "TestModem.h"
#include "TestModem2.h"

// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

using namespace std;
using namespace radlib;

// ------ Data Area -------

const unsigned int sampleFreq = 2000;
const uint16_t lowFreq = 50;
const unsigned int samplesPerSymbol = 60;
const unsigned int usPerSymbol = (1000000 / sampleFreq) * samplesPerSymbol;
const unsigned int markFreq = 667;
const unsigned int spaceFreq = 600;

// Here we can inject a tuning error to show that the demodulator will
// still find the signal.
const unsigned int tuningErrorHz = 0;

// Reserve space for modulation
const unsigned int S = ((30 * 30) + 30) * samplesPerSymbol;
static float samples[S];

// The size of the FFT used for frequency acquisition
const uint16_t log2fftN = 9;
const uint16_t fftN = 1 << log2fftN;

// Allocate 1 second of sample data
TestDemodulatorListener::Sample SampleArea[2000];

int main(int, const char**) {

    memset((void*)samples, 0, sizeof(samples));

    cout << "SCAMP Modem Demonstration 7a" << endl;
    cout << "  Sample Freq    : " << sampleFreq << endl;
    cout << "  Low Freq       : " << lowFreq << endl;
    cout << "  Mark           : " << markFreq + tuningErrorHz << endl;
    cout << "  Space          : " << spaceFreq + tuningErrorHz << endl;
    cout << "  Samples/Symbol : " << samplesPerSymbol << endl;

    // This is the modem used for the demonstration.  Samples
    // are written to a memory buffer. 
    //
    // Note that we have applied a DC bias here to ensure that this doesn't 
    // create a problem anywhere in the decoder.
    //
    // Note that we have applied random noise to the signal to ensure that 
    // this doesn't create a problem.
    TestModem2 modem2(samples, S, sampleFreq, 
        markFreq + tuningErrorHz, spaceFreq + tuningErrorHz, 0.3, 0.1, 0.07);

    // This is a modem that is used to capture the data for printing.
    int8_t printSamples[34 * 30];
    TestModem printModem(printSamples, sizeof(printSamples), 1);

    const char* testMessage1 = "DE KC1FSZ, GOOD MORNING";
    const char* testMessage2 = "73S, HAVE A GOOD DAY";
    
    // =========================================================================
    // Make a message, encode it, and then recover it. In this test we are 
    // using the SCAMP FSK (33.3 bits per second) format.  
    {        
        // Encode the message.  This leads to about 25K samples.
        Frame30 frames[32];

        unsigned int frameCount1 = encodeString(testMessage1, frames, 45, true);
        cout << "Used frames " << frameCount1 << endl;
        assertm(frameCount1 < 32, "FRAME COUNT");
    
        // This silence is 30 symbols, or 30 * 60 = 1800 samples long
        for (unsigned int i = 0; i < 30; i++)
            modem2.sendSilence(usPerSymbol);

        for (unsigned int i = 0; i < frameCount1; i++) {
            frames[i].transmit(modem2, usPerSymbol);
            frames[i].transmit(printModem, usPerSymbol);
            // TEST: Double-up one of the frames just to show that the decoder will ignore it
            if (i == 6) {
                frames[i].transmit(modem2, usPerSymbol);
                frames[i].transmit(printModem, usPerSymbol);            
            }
        }
        // Trailing silence
        for (unsigned int i = 0; i < 30; i++) {
            modem2.sendSilence(usPerSymbol);
        }

        unsigned int frameCount2 = encodeString(testMessage2, frames, 45, true);
        cout << "Used frames " << frameCount2 << endl;
        assertm(frameCount2 < 32, "FRAME COUNT");

        for (unsigned int i = 0; i < frameCount2; i++) {
            frames[i].transmit(modem2, usPerSymbol);
        }
        // Trailing silence
        //for (unsigned int i = 0; i < 30; i++) {
        //    modem2.sendSilence();
        //}
    }

    // Display
    {
        cout << endl << "Sending these frames:" << endl;
        for (uint16_t i = 0; i < printModem.getSamplesUsed(); i++) {
            if (i % 30 == 0) {
                cout << endl;
            }
            if (printSamples[i] == 1) {
                cout << "1";
            } else if (printSamples[i] == -1) {
                cout << "0";
            } else {
                cout << "?";
            }
        }
        cout << endl << endl;
    }

    // Now decode without any prior knowledge of the frequency or phase
    // of the transmitter.
    {
        TestDemodulatorListener testListener(cout, SampleArea, 2000);
        testListener.setTriggerMode(TestDemodulatorListener::TriggerMode::ON_LOCK);

        // Space for the demodulator to work in (no dynamic memory allocation!)
        q15 trigTable[fftN];
        q15 window[fftN];
        q15 buffer[fftN];
        cq15 fftResult[fftN];

        SCAMPDemodulator demod(sampleFreq, lowFreq, log2fftN,
            trigTable, window, fftResult, buffer);
        demod.setListener(&testListener);

        // Walk through the data one byte at a time.  We do something extra
        // each time we have processed a complete block.
        uint32_t samplePtr = 0;
        while (samplePtr < modem2.getSamplesUsed()) {            
            const q15 sample = f32_to_q15(samples[samplePtr++]);
            demod.processSample(sample);
        }

        cout << "FRAMES  : " << demod.getFrameCount() << endl;
        cout << "PLL INT : " << demod.getPLLIntegration() << endl;
        cout << "PLL ERR : " << 100.0f * demod.getClockRecoveryPhaseError() << " %" << endl;
        cout << "LAST DC : " << demod.getLastDCPower() << endl;
        cout << "MARK HZ : " << demod.getMarkFreq() << endl;
        cout << "MESSAGE : " << testListener.getMessage() << endl;
        //assertm(testListener.getMessage() == "DE KC1FSZ, GOOD MORNING73S, HAVE A GOOD DAY", 
        //    "Message Failure");

        // Demo the trace
        //testListener.dumpSamples(cout);
    }
}

// FIRST DATA FRAME
//
// 01111 10000 01111 01000 01010 10001
// 01234 56789 01234 56789 01234 56789

// DATA SYNC IN SYMBOL 89 = 60 + 24
