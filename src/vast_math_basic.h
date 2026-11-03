#ifndef VAST_MATH_BASIC_H
#define VAST_MATH_BASIC_H

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

#define VAST_PI 3.14159265358979323846f
#define VAST_PI2 6.28318530717958647692f
#define VAST_PI_HALF 1.57079632679489661923f
#define VAST_TAU 6.283185307f
#define VAST_SQRT3 1.7320508f
#define VAST_DEG_TO_RAD(deg) ((deg) * (VAST_PI / 180.0f))
#define VAST_RAD_TO_DEG(rad) ((rad) * (180.0f / VAST_PI))

#define VAST_SIN_LUT_SIZE 256

static f32 vast_sin_lut[VAST_SIN_LUT_SIZE] = {
    0.0000f, 0.0245f, 0.0491f, 0.0736f, 0.0980f, 0.1224f, 0.1467f, 0.1710f,
    0.1951f, 0.2191f, 0.2430f, 0.2667f, 0.2903f, 0.3137f, 0.3369f, 0.3599f,
    0.3827f, 0.4052f, 0.4276f, 0.4496f, 0.4714f, 0.4929f, 0.5141f, 0.5350f,
    0.5556f, 0.5758f, 0.5957f, 0.6152f, 0.6344f, 0.6532f, 0.6716f, 0.6895f,
    0.7071f, 0.7242f, 0.7409f, 0.7572f, 0.7730f, 0.7883f, 0.8032f, 0.8176f,
    0.8315f, 0.8449f, 0.8577f, 0.8701f, 0.8819f, 0.8932f, 0.9040f, 0.9142f,
    0.9239f, 0.9330f, 0.9415f, 0.9495f, 0.9569f, 0.9638f, 0.9700f, 0.9757f,
    0.9808f, 0.9853f, 0.9892f, 0.9925f, 0.9952f, 0.9973f, 0.9988f, 0.9997f,
    1.0000f, 0.9997f, 0.9988f, 0.9973f, 0.9952f, 0.9925f, 0.9892f, 0.9853f,
    0.9808f, 0.9757f, 0.9700f, 0.9638f, 0.9569f, 0.9495f, 0.9415f, 0.9330f,
    0.9239f, 0.9142f, 0.9040f, 0.8932f, 0.8819f, 0.8701f, 0.8577f, 0.8449f,
    0.8315f, 0.8176f, 0.8032f, 0.7883f, 0.7730f, 0.7572f, 0.7409f, 0.7242f,
    0.7071f, 0.6895f, 0.6716f, 0.6532f, 0.6344f, 0.6152f, 0.5957f, 0.5758f,
    0.5556f, 0.5350f, 0.5141f, 0.4929f, 0.4714f, 0.4496f, 0.4276f, 0.4052f,
    0.3827f, 0.3599f, 0.3369f, 0.3137f, 0.2903f, 0.2667f, 0.2430f, 0.2191f,
    0.1951f, 0.1710f, 0.1467f, 0.1224f, 0.0980f, 0.0736f, 0.0491f, 0.0245f,
    0.0000f, -0.0245f, -0.0491f, -0.0736f, -0.0980f, -0.1224f, -0.1467f, -0.1710f,
    -0.1951f, -0.2191f, -0.2430f, -0.2667f, -0.2903f, -0.3137f, -0.3369f, -0.3599f,
    -0.3827f, -0.4052f, -0.4276f, -0.4496f, -0.4714f, -0.4929f, -0.5141f, -0.5350f,
    -0.5556f, -0.5758f, -0.5957f, -0.6152f, -0.6344f, -0.6532f, -0.6716f, -0.6895f,
    -0.7071f, -0.7242f, -0.7409f, -0.7572f, -0.7730f, -0.7883f, -0.8032f, -0.8176f,
    -0.8315f, -0.8449f, -0.8577f, -0.8701f, -0.8819f, -0.8932f, -0.9040f, -0.9142f,
    -0.9239f, -0.9330f, -0.9415f, -0.9495f, -0.9569f, -0.9638f, -0.9700f, -0.9757f,
    -0.9808f, -0.9853f, -0.9892f, -0.9925f, -0.9952f, -0.9973f, -0.9988f, -0.9997f,
    -1.0000f, -0.9997f, -0.9988f, -0.9973f, -0.9952f, -0.9925f, -0.9892f, -0.9853f,
    -0.9808f, -0.9757f, -0.9700f, -0.9638f, -0.9569f, -0.9495f, -0.9415f, -0.9330f,
    -0.9239f, -0.9142f, -0.9040f, -0.8932f, -0.8819f, -0.8701f, -0.8577f, -0.8449f,
    -0.8315f, -0.8176f, -0.8032f, -0.7883f, -0.7730f, -0.7572f, -0.7409f, -0.7242f,
    -0.7071f, -0.6895f, -0.6716f, -0.6532f, -0.6344f, -0.6152f, -0.5957f, -0.5758f,
    -0.5556f, -0.5350f, -0.5141f, -0.4929f, -0.4714f, -0.4496f, -0.4276f, -0.4052f,
    -0.3827f, -0.3599f, -0.3369f, -0.3137f, -0.2903f, -0.2667f, -0.2430f, -0.2191f,
    -0.1951f, -0.1710f, -0.1467f, -0.1224f, -0.0980f, -0.0736f, -0.0491f, -0.0245f};

VAST_API VAST_INLINE f32 vast_sinf(f32 x)
{
    f32 index, frac;
    i32 i, i2;

    x -= VAST_PI2 * (f32)((i32)(x * (1.0f / VAST_PI2)));

    if (x < 0)
    {
        x += VAST_PI2;
    }

    index = x * (VAST_SIN_LUT_SIZE / VAST_PI2);
    i = (i32)index;
    frac = index - (f32)i;

    i &= (VAST_SIN_LUT_SIZE - 1);
    i2 = (i + 1) & (VAST_SIN_LUT_SIZE - 1);

    return vast_sin_lut[i] + frac * (vast_sin_lut[i2] - vast_sin_lut[i]);
}

VAST_API VAST_INLINE f32 vast_cosf(f32 x)
{
    return vast_sinf(x + VAST_PI_HALF);
}

VAST_API VAST_INLINE f32 vast_sinf_snorm(f32 a)
{
    static f32 a0 = +1.91059300966915117e-31f;
    static f32 a1 = +1.00086760103908896f;
    static f32 a2 = -1.21276126894734565e-2f;
    static f32 a3 = -1.38078780785773762e-1f;
    static f32 a4 = -2.67353392911981221e-2f;
    static f32 a5 = +2.08026600266304389e-2f;
    static f32 a6 = -3.03996055049204407e-3f;
    static f32 a7 = +1.38235642404333740e-4f;
    return a0 + a * (a1 + a * (a2 + a * (a3 + a * (a4 + a * (a5 + a * (a6 + a * a7))))));
}

VAST_API VAST_INLINE f32 vast_cosf_snorm(f32 a)
{
    static f32 a0 = +1.00238601909309722f;
    static f32 a1 = -3.81919947353040024e-2f;
    static f32 a2 = -3.94382342128062756e-1f;
    static f32 a3 = -1.18134036025221444e-1f;
    static f32 a4 = +1.07123798512170878e-1f;
    static f32 a5 = -1.86637164165180873e-2f;
    static f32 a6 = +9.90140908664079833e-4f;
    static f32 a7 = -5.23022132118824778e-14f;
    return a0 + a * (a1 + a * (a2 + a * (a3 + a * (a4 + a * (a5 + a * (a6 + a * a7))))));
}

VAST_API VAST_INLINE f32 vast_tanf(f32 x)
{
    return (vast_sinf(x) / vast_cosf(x));
}

VAST_API VAST_INLINE f32 vast_floorf(f32 x)
{
    i32 i = (i32)x;
    return (x < (f32)i) ? (f32)(i - 1) : (f32)i;
}

VAST_API VAST_INLINE f32 vast_remapf(f32 v, f32 in_min, f32 in_max, f32 out_min, f32 out_max)
{
    f32 t;

    if (in_max == in_min)
    {
        return out_min;
    }

    t = (v - in_min) / (in_max - in_min);

    if (t < 0.0f)
    {
        t = 0.0f;
    }

    if (t > 1.0f)
    {
        t = 1.0f;
    }

    return out_min + t * (out_max - out_min);
}

#endif /* VAST_MATH_BASIC_H */