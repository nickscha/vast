#ifndef VAST_MATH_BASIC_SCALAR_H
#define VAST_MATH_BASIC_SCALAR_H

#include "vast_types.h"

/* #############################################################################
 * # [SECTION] Basic Math (Scalar)
 * #############################################################################
 */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4699) /* MSVC-specific aliasing warning */
#endif
VAST_API VAST_INLINE f32 vast_invsqrtf(f32 number)
{
    union
    {
        f32 f;
        i32 i;
    } conv;

    f32 x2, y;
    const f32 threehalfs = 1.5F;

    x2 = number * 0.5F;
    conv.f = number;
    conv.i = 0x5f3759df - (conv.i >> 1); /* Magic number for approximation */
    y = conv.f;
    y = y * (threehalfs - (x2 * y * y)); /* One iteration of Newton's method */

    return (y);
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

VAST_API VAST_INLINE f32 vast_sqrtf(f32 x)
{
    return x * vast_invsqrtf(x);
}

VAST_API VAST_INLINE f32 vast_absf(f32 x)
{
    union
    {
        f32 f;
        u32 i;
    } conv;

    conv.f = x;
    conv.i &= 0x7FFFFFFF; /* Clear the sign bit */

    return conv.f;
}

VAST_API VAST_INLINE f32 vast_minf(f32 a, f32 b)
{
    return (a < b) ? a : b;
}

VAST_API VAST_INLINE f32 vast_maxf(f32 a, f32 b)
{
    return (a > b) ? a : b;
}

VAST_API VAST_INLINE f32 vast_clampf(f32 x, f32 a, f32 b)
{
    f32 x1 = (x < a) ? a : x;
    return (x1 > b) ? b : x1;
}

VAST_API VAST_INLINE f32 vast_roundf(f32 v)
{
    /* round-to-nearest */
    return (v >= 0.0f) ? (f32)(i32)(v + 0.5f) : (f32)(i32)(v - 0.5f);
}

VAST_API VAST_INLINE f32 vast_lerpf(f32 a, f32 b, f32 t)
{
    return a + (b - a) * t;
}

VAST_API VAST_INLINE f32 vast_ceilf(f32 x)
{
    i32 i = (i32)x;
    return (x > (f32)i) ? (f32)(i + 1) : (f32)i;
}

VAST_API VAST_INLINE f32 vast_fmodf(f32 x, f32 y)
{
    f32 quotient;
    i32 i_quotient;
    f32 result;

    if (y == 0.0f)
    {
        return 0.0f;
    }

    /* 1. Calculate how many times y goes into x */
    quotient = x / y;

    /* 2. Truncate to integer */
    i_quotient = (i32)quotient;

    /* 3. Subtract the integer part of y from x */
    result = x - ((f32)i_quotient * y);

    return result;
}

#endif /* VAST_MATH_BASIC_SCALAR_H */