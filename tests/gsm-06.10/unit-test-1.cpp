#include <iostream>
#include "../../gsm-06.10/Encoder.h"

using namespace std;
using namespace radlib;

int main(int, const char**) {
    
    Encoder e;
    int16_t input[160];
    uint8_t output[33];
    e.encode(input, output);

}