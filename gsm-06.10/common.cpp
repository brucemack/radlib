/**
 * Copyright (C) 2024, Bruce MacKinnon KC1FSZ
 * 
 * NOT FOR COMMERCIAL USE.
 */
#include "common.h"

namespace radlib {

/**
 * Performs the addition (var1+var2) with overflow control and saturation; the result is set at +32767
 * when overflow occurs or at -32768 when underflow occurs.
 */
int16_t add(int16_t var1, int16_t var2) {
    return 0;
}

/**
 * Performs the subtraction (var1-var2) with overflow control and saturation; the result is set at
 * +32767 when overflow occurs or at -32768 when underflow occurs.
 */
int16_t sub(int16_t var1, int16_t var2) {
    return 0;
}

/**
 * Performs the multiplication of var1 by var2 and gives a 16 bits result which is scaled i.e.
 * mult(var1,var2 ) = (var1 times var2) >> 15 and mult(-32768, -32768) = 32767.
 */
int16_t mult(int16_t var1, int16_t var2) {
    return 0;
}

/**
 * Same as mult but with rounding i.e. mult_r( var1, var2 ) = ( (var1 times var2) + 16384 ) >> 15
 * and mult_r( -32768, -32768 ) = 32767
 */
int16_t mult_r(int16_t var1, int16_t var2) {
    return 0;
}

/** 
 * Absolute value of var1; abs(-32768) = 32767
 */ 
int16_t abs(int16_t var1) {
    return 0;
}

/**
 * div produces a result which is the fractional integer division of var1 by var2; var1 and var2 shall
 * be positive and var2 shall be greater or equal to var1; The result is positive (leading bit equal to 0)
 * and truncated to 16 bits. if var1 == var2 then div( var1, var2 ) = 32767
 */
int16_t div(int16_t var1, int16_t var2) {
    return 0;
}

/** 
 * L_mult is a 32 bit result for the multiplication of var1 times var2 with a one bit shift left.
 * L_mult( var1, var2 ) = ( var1 times var2 ) << 1. The condition L_mult (-32768, -32768 ) does not
 * occur in the [GSM] algorithm.
 */
int32_t L_mult(int32_t var1, int32_t var2) {
    return 0;
}

/**
 * 32 bits addition of two 32 bits variables (L_var1 + L_var2) with overflow control and
 * saturation; the result is set at 2147483647 when overflow occurs and at -2147483648 when
 * underflow occurs.
 */
int32_t L_add(int32_t L_var1, int32_t L_var2) {
    return 0;
}

/**
 * 32 bits subtraction of two 32 bits variables (L_var1 - L_var2) with overflow control and
 * saturation; the result is set at 2147483647 when overflow occurs and at -2147483648 when
 * underflow occurs.
*/
int32_t L_sub(int32_t L_var1, int32_t L_var2) {
    return 0;
}

}
