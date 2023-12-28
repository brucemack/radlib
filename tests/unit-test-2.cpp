#include <iostream>
#include <cassert>

#include "../util/WindowAverage.h"

using namespace std;
using namespace radlib;


int main(int,const char**) {

    int16_t windowArea[4];
    WindowAverage avg(2, windowArea);
    assert(1 == avg.sample(4));
    assert(2 == avg.sample(4));
    assert(3 == avg.sample(4));
    assert(4 == avg.sample(4));
    assert(3 == avg.sample(0));
    assert(2 == avg.sample(0));
    assert(1 == avg.sample(0));
    assert(0 == avg.sample(0));
    assert(-1 == avg.sample(-4));
}


