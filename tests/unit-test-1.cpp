#include <iostream>
#include "TestFSKModulator.h"
#include "../rtty/BaudotEncoder.h"

using namespace std;
using namespace radlib;

uint32_t symbolUs = 21978;

int main(int,const char**) {
    cout << "Hello RTTY" << endl;
    TestFSKModulator mod;
    transmitBaudot("CQCQ DE KC1FSZ", mod, symbolUs);
    //transmitBaudot("CQ", mod, symbolUs);
    cout << endl;
}


