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
#include "MorseEncoder.h"

using namespace std;

namespace radlib {

static void silence(FSKModulator& mod, uint16_t dots, uint16_t speed) {
    uint32_t dot_ms = 50;
    uint32_t t = (uint32_t)dots * dot_ms * 1000;
    mod.sendSilence(t);
}

static void dot(FSKModulator& mod, uint16_t speed, bool last = false) {
    uint32_t dot_ms = 50;
    uint32_t t = dot_ms * 1000;
    mod.sendMark(t);
    if (!last)
        silence(mod, 1, speed);
}

static void dash(FSKModulator& mod, uint16_t speed, bool last = false) {
    uint32_t dot_ms = 50;
    uint32_t t = 3L * dot_ms * 1000;
    mod.sendMark(t);
    if (!last)
        silence(mod, 1, speed);
}

void send_morse_char(char ch, FSKModulator& mod, uint16_t speed) {

    switch (ch) 
    {
        case 'A': 
            dot(mod, speed);
            dash(mod, speed, true);
            break;
        case 'B': 
            dash(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case 'C': 
            dash(mod, speed);
            dot(mod, speed);
            dash(mod, speed);
            dot(mod, speed, true);
            break;
        case 'D': 
            dash(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case 'E': 
            dot(mod, speed, true);
            break;
        case 'F': 
            dot(mod, speed);
            dot(mod, speed);
            dash(mod, speed);
            dot(mod, speed, true);
            break;
        case 'G': 
            dash(mod, speed);
            dash(mod, speed);
            dot(mod, speed, true);
            break;
        case 'H': 
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case 'I': 
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case 'J': 
            dot(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed, true);
            break;
        case 'K': 
            dash(mod, speed);
            dot(mod, speed);
            dash(mod, speed, true);
            break;
        case 'L': 
            dot(mod, speed);
            dash(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case 'M': 
            dash(mod, speed);
            dash(mod, speed, true);
            break;
        case 'N': 
            dash(mod, speed);
            dot(mod, speed, true);
            break;
        case 'O': 
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed, true);
            break;
        case 'P': 
            dot(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dot(mod, speed, true);
            break;
        case 'Q': 
            dash(mod, speed);
            dash(mod, speed);
            dot(mod, speed);
            dash(mod, speed, true);
            break;
        case 'R': 
            dot(mod, speed);
            dash(mod, speed);
            dot(mod, speed, true);
            break;
        case 'S': 
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case 'T': 
            dash(mod, speed, true);
            break;
        case 'U': 
            dot(mod, speed);
            dot(mod, speed);
            dash(mod, speed, true);
            break;
        case 'V': 
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dash(mod, speed, true);
            break;
        case 'W': 
            dot(mod, speed);
            dash(mod, speed);
            dash(mod, speed, true);
            break;
        case 'X': 
            dash(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dash(mod, speed, true);
            break;
        case 'Y': 
            dash(mod, speed);
            dot(mod, speed);
            dash(mod, speed);
            dash(mod, speed, true);
            break;
        case 'Z': 
            dash(mod, speed);
            dash(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case '1': 
            dot(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed, true);
            break;
        case '2': 
            dot(mod, speed);
            dot(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed, true);
            break;
        case '3': 
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dash(mod, speed);
            dash(mod, speed, true);
            break;
        case '4': 
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dash(mod, speed, true);
            break;
        case '5': 
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case '6': 
            dash(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case '7': 
            dash(mod, speed);
            dash(mod, speed);
            dot(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case '8': 
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dot(mod, speed);
            dot(mod, speed, true);
            break;
        case '9': 
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dot(mod, speed, true);
            break;
        case '0': 
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed);
            dash(mod, speed, true);
            break;
    }
}

void send_morse(const char* s, FSKModulator& mod, uint16_t speed) {
    cout << "Morse message: " << s << endl;
    uint16_t i = 0;
    for (i = 0; s[i] != 0; i++) {
        if (s[i] == ' ') {
            // NOTE: WE ALREADY SENT 3 AFTER LAST CHAR
            silence(mod, 4, speed);
        } else {
            send_morse_char(s[i], mod, speed);
            silence(mod, 3, speed);
        }
    }
    cout << "DONE" << endl;
}

}
