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
#include "BaudotDecoder.h"

using namespace std;
using namespace radlib;

namespace radlib {

const uint8_t BAUDOT_LTRS = 31;
const uint8_t BAUDOT_FIGS = 27;

const char BAUDOT_TO_ASCII_MAP[32][2] = {
    {   0, 0   },
    { 'E', '3' },
    { '\n', '\n' },
    { 'A', '-' },
    { ' ', ' ' },
    { 'S', '\x07' },
    { 'I', '8' },
    { 'U', '7' },
    { '\r', '\r' },
    { 'D', '$' },
    { 'R', '4' },
    { 'J', '\'' },
    { 'N', ',' },
    { 'F', '!' },
    { 'C', ':' },
    { 'K', '(' },
    { 'T', '5' },
    { 'Z', '"' },
    { 'L', ')' },
    { 'W', '2' },
    { 'H', '#' },
    { 'Y', '6' },
    { 'P', '0' },
    { 'Q', '1' },
    { 'O', '9' },
    { 'B', '?' },
    { 'G', '&' },
    {   0, 0   },    // FIGS
    { 'M', '.' },
    { 'X', '/' },
    { 'V', ';' },
    {   0, 0,  }    // LTRS
};
}
