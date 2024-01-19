#include <cassert>
#include <iostream>
#include <bitset>
#include "../../gsm-06.10/common.h"
#include "../../gsm-06.10/Parameters.h"
#include "../../gsm-06.10/Encoder.h"

using namespace radlib;

static void math_tests() {
    // Add with saturation
    {
        // Check overflow cases
        int16_t a = 32000, b = 1000;
        assert(sizeof(a) == 2);
        assert(add(a, b) == 32767);
        a = -32000;
        b = -1000;
        assert(add(a, b) == -32768);

        a = -1;
        assert((uint16_t)a == 0b1111111111111111);
        a = -2;
        assert((uint16_t)a == 0b1111111111111110);
        //std::bitset<16> y(a);
        //cout << y << endl;
        a = -32768;
        assert((uint16_t)a == 0b1000000000000000);
        a = 32767;
        assert((uint16_t)a == 0b0111111111111111);
    }

    // Subtraction with saturation
    {
        // Check overflow cases
        int16_t a = -32000, b = 1000;
        assert(sub(a, b) == -32768);
        a = 32000;
        b = -1000;
        assert(sub(a, b) == 32767);
    }

    // Mult
    {
        // -1 * 0.5 (approximately, but slightly smaller)
        int16_t a = -32768, b = 32767 / 2;
        assert(mult(a, b) == -16383);

        // -1 * -1
        a = -32768;
        b = -32768;
        // Make sure we wrap here in this special case
        assert(mult(a, b) == 32767);

        // -1 * 0.99999999....
        a = -32768;
        b = 32767;
        // Should be very close to -1
        assert(mult(a, b) == -32767);

        // 0.999999.... * -1
        a = 32767;
        b = -32768;
        // Should be very close to -1
        assert(mult(a, b) == -32767);
    }

    // Mult with rounding
    {
        // -1 * 0.5 (approximately, but slightly smaller)
        int16_t a = -32768, b = 32767 / 2;
        assert(mult_r(a, b) == -16383);

        // -1 * 0.5 (approximately, but slightly larger). 
        a = -32768;
        b = 32768 / 2;
        assert(mult_r(a, b) == -16384);

        // 0.5 * 0.5 
        a = 16384;
        b = 16384;
        assert(mult_r(a, b) == 8192);

        // -0.5 * 0.5 
        a = -16384;
        b = 16384;
        assert(mult_r(a, b) == -8192);

        // -1 * -1
        a = -32768;
        b = -32768;
        // Make sure we wrap here in this special case
        assert(mult_r(a, b) == 32767);

        // -1 * 0.99999999....
        a = -32768;
        b = 32767;
        // Should be very close to -1
        assert(mult_r(a, b) == -32767);

        // 0.999999.... * -1
        a = 32767;
        b = -32768;
        // Should be very close to -1
        assert(mult_r(a, b) == -32767);
    }
    
    // Abs
    {
        assert(s_abs(-32767) == 32767);
        assert(s_abs(32767) == 32767);
        // Saturation case
        assert(s_abs(-32768) == 32767);
    }

    // div
    {
        int16_t n, d;
        n = 32768 / 4;
        d = 32768 / 2;
        // 0.25 / 0.5 should equal 0.5
        assert(div(n, d) == 32768 / 2);
        // 0.25 / 0.25 should saturate to +32767
        assert(div(n, n) == 32767);
    }

    // 32-bit addition
    {
        int32_t a, b;
        a = 2147483647;
        b = 1;
        // We should saturate here
        assert(L_add(a, b) == a);

        a = -2147483648;
        b = -1;
        // We should saturate here
        assert(L_add(a, b) == a);
    }

    // 32-bit subtraction
    {
        int32_t a, b;
        a = 2147483647;
        b = -1;
        // We should saturate here
        assert(L_sub(a, b) == a);

        a = -2147483648;
        b = 1;
        // We should saturate here
        assert(L_sub(a, b) == a);
    }

    // Normalization 
    {
        int32_t a;
        a = 2147483647;
        assert(norm(a) == 0);
        a = 1073741825;
        assert(norm(a) == 0);
        a = 1073741824;
        assert(norm(a) == 0);
        a = 1073741823;
        assert(norm(a) == 1);

        a = -2147483648;
        assert(norm(a) == 0);
        a = -2147483647;
        assert(norm(a) == 0);
        a = -1073741825;
        assert((uint32_t)a == 0b10111111111111111111111111111111);
        assert(norm(a) == 0);
        a = -1073741824;
        assert((uint32_t)a == 0b11000000000000000000000000000000);
        assert(norm(a) == 0);
        a = -1073741823;
        assert((uint32_t)a == 0b11000000000000000000000000000001);
        assert(norm(a) == 1);
    }

}

int main(int, const char**) {
    
    math_tests();

    assert(sizeof(Parameters) == 76);

    Encoder e;
    int16_t input[160];
    Parameters output;
    e.encode(input, &output);

}