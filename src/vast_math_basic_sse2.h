#ifndef VAST_MATH_BASIC_SSE2_H
#define VAST_MATH_BASIC_SSE2_H

#include "vast_types.h"

/* #############################################################################
 * # [SECTION] Basic Math (SSE2)
 * #############################################################################
 */
#include <emmintrin.h>

VAST_API VAST_INLINE f32 vast_invsqrtf(f32 number)
{
    /* Use the hardware reciprocal square root instruction */
    __m128 v = _mm_set_ss(number);
    __m128 rsqrt = _mm_rsqrt_ss(v);

    /* Newton-Raphson iteration for higher precision */
    __m128 half = _mm_set_ss(0.5f);
    __m128 three_halfs = _mm_set_ss(1.5f);
    __m128 muls = _mm_mul_ss(_mm_mul_ss(v, half), _mm_mul_ss(rsqrt, rsqrt));
    rsqrt = _mm_mul_ss(rsqrt, _mm_sub_ss(three_halfs, muls));

    return _mm_cvtss_f32(rsqrt);
}

VAST_API VAST_INLINE f32 vast_sqrtf(f32 x)
{
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
}

VAST_API VAST_INLINE f32 vast_absf(f32 x)
{
    __m128 v = _mm_set_ss(x);
    __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
    return _mm_cvtss_f32(_mm_and_ps(v, mask));
}

VAST_API VAST_INLINE f32 vast_minf(f32 a, f32 b)
{
    __m128 va = _mm_set_ss(a);
    __m128 vb = _mm_set_ss(b);
    return _mm_cvtss_f32(_mm_min_ss(va, vb));
}

VAST_API VAST_INLINE f32 vast_maxf(f32 a, f32 b)
{
    __m128 va = _mm_set_ss(a);
    __m128 vb = _mm_set_ss(b);
    return _mm_cvtss_f32(_mm_max_ss(va, vb));
}

VAST_API VAST_INLINE f32 vast_clampf(f32 x, f32 a, f32 b)
{
    __m128 vx = _mm_set_ss(x);
    __m128 va = _mm_set_ss(a);
    __m128 vb = _mm_set_ss(b);
    return _mm_cvtss_f32(_mm_min_ss(_mm_max_ss(vx, va), vb));
}

VAST_API VAST_INLINE f32 vast_roundf(f32 v)
{
    __m128 v_in;
    __m128i i_round;
    __m128 v_out;

    v_in = _mm_set_ss(v);
    i_round = _mm_cvtps_epi32(v_in);
    v_out = _mm_cvtepi32_ps(i_round);

    return _mm_cvtss_f32(v_out);
}

VAST_API VAST_INLINE f32 vast_lerpf(f32 a, f32 b, f32 t)
{
    __m128 va, vb, vt, res;

    va = _mm_set_ss(a);
    vb = _mm_set_ss(b);
    vt = _mm_set_ss(t);

    res = _mm_add_ss(va, _mm_mul_ss(_mm_sub_ss(vb, va), vt));

    return _mm_cvtss_f32(res);
}

VAST_API VAST_INLINE f32 vast_ceilf(f32 x)
{
    __m128 v = _mm_set_ss(x);
    __m128i i = _mm_cvttps_epi32(v);
    __m128 truncated = _mm_cvtepi32_ps(i);
    __m128 mask = _mm_cmplt_ss(truncated, v);
    __m128 one = _mm_set_ss(1.0f);
    __m128 adjustment = _mm_and_ps(mask, one);

    return _mm_cvtss_f32(_mm_add_ss(truncated, adjustment));
}

VAST_API VAST_INLINE f32 vast_fmodf(f32 x, f32 y)
{
    __m128 vx, vy, v_quot, v_trunc, v_res;
    __m128i i_trunc;

    vx = _mm_set_ss(x);
    vy = _mm_set_ss(y);

    v_quot = _mm_div_ss(vx, vy);
    i_trunc = _mm_cvttps_epi32(v_quot);
    v_trunc = _mm_cvtepi32_ps(i_trunc);
    v_res = _mm_sub_ss(vx, _mm_mul_ss(v_trunc, vy));

    return _mm_cvtss_f32(v_res);
}

#endif /* VAST_MATH_BASIC_SSE2_H */