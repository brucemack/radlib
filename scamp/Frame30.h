/*
Copyright (C) 2024 - Bruce MacKinnon KC1FSZ

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
#ifndef _Frame30_h
#define _Frame30_h

#include "CodeWord24.h"
#include "../util/FSKModulator.h"

namespace radlib {

    /**
     * Class represents a 30 bit SCAMP frame. Our goal is to use 
     * strongly-typed concepts and to minimize the use of 
     * typedefs and ad-hoc integers.
    */
    class Frame30 {
    public:

        static uint32_t MASK30LSB;

        // Start of transmission frame, contains edges to synchronize with
        static Frame30 START_FRAME;
        // Sync frame 
        static Frame30 SYNC_FRAME;
        // NOTE: NOT A VALID FRAME - JUST USED FOR SYNC TESTING
        static Frame30 ZERO_FRAME;
        // NOTE: NOT A VALID FRAME - JUST USED FOR SYNC TESTING
        static Frame30 ALT_FRAME;

        /**
         * Converts a CodeWord24 into a complete frame by adding 
         * the compliment bits.
         */
        static Frame30 fromCodeWord24(CodeWord24 cw);

        /**
         * A convenience function that rolls together all of steps needed
         * to create a SCAMP frame from two ASCII characters.
         */
        static Frame30 fromTwoAsciiChars(char a, char b);

        /** 
         * Correlates the first 30 LSB bits of the 32 bit number.
         */
        static int32_t correlate30(uint32_t a, uint32_t b);

        /**
         * MSB is the first to be sent.
         */
        static uint32_t arrayToInt32(uint8_t a[]);

        /**
         * Utility function that decodes an array of marks/spaces
         * an creates a frame. The first tone received is in 
         * the [0] location and the last tone received is in the 
         * [29] location.
         */
        static Frame30 decodeFromTones(uint8_t tones[]);

        Frame30();
        Frame30(uint32_t raw);

        uint32_t getRaw() const;

        /**
         * Returns true if there are 6 valid compliments in the frame;
         */
        bool isValid() const;

        /**
         * Extracts the 24 bit code-word from a frame.
         */
        CodeWord24 toCodeWord24() const;

        /**
         * Returns the number of compliment sets.  Expected value is 6.
         */
        unsigned int getComplimentCount() const;

        /**
         * Transmits frame contents.
         */
        void transmit(FSKModulator& modulator, uint32_t symbolDurationUs, int16_t errorMs = 0);

    private:

        uint32_t _raw;
    };
}

#endif
