#include <iostream>
#include "TestFSKModulator.h"
#include "../util/DataListener.h"
#include "../rtty/BaudotEncoder.h"
#include "../rtty/BaudotDecoder.h"

using namespace std;
using namespace radlib;

// This is 45.45 baud
uint32_t symbolUs = 22002;

// Test listener
class Listener : public DataListener {
public:
    void received(char asciiChar) {
        cout << asciiChar;
        cout.flush();
    }
};

static void run(BaudotDecoder& dec, uint8_t symbol, uint16_t copies) {
    for (uint16_t i = 0; i < copies; i++) {
        dec.processSample(symbol);
    }
}

int main(int,const char**) {
    
    cout << "Hello RTTY" << endl;
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

}


