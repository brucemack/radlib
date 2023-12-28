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


