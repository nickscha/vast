#ifndef VAST_MATH_LINEAR_ALGEBRA_H
#define VAST_MATH_LINEAR_ALGEBRA_H

#include "vast_math_basic.h"

/* #############################################################################
 * # [SECTION] Linear Algebra Math (SIMD Detection)
 * #############################################################################
 */
#ifdef VAST_DISABLE_SIMD
#include "vast_math_linear_algebra_scalar.h"
#elif defined(VAST_ARCH_X64)
#include "vast_math_linear_algebra_sse2.h"
#endif

typedef struct vast_vec2
{
    f32 x;
    f32 y;

} vast_vec2;

VAST_API VAST_INLINE vast_vec2 vast_vec2_init(f32 x, f32 y)
{
    vast_vec2 result;

    result.x = x;
    result.y = y;

    return result;
}

VAST_API VAST_INLINE f32 vast_vec2_dot(vast_vec2 a, vast_vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

typedef struct vast_vec4
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;

} vast_vec4;

VAST_API VAST_INLINE vast_mat4x4 vast_mat4x4_perspective(f32 fov, f32 aspectRatio, f32 zNear, f32 zFar)
{
    f32 f = 1.0f / vast_tanf(fov * 0.5f);
    f32 fn = 1.0f / (zNear - zFar);

    vast_mat4x4 result = vast_mat4x4_zero;

    result.e[VAST_MAT4X4_AT(0, 0)] = f / aspectRatio;
    result.e[VAST_MAT4X4_AT(1, 1)] = f;
    result.e[VAST_MAT4X4_AT(2, 2)] = (zNear + zFar) * fn;
    result.e[VAST_MAT4X4_AT(2, 3)] = (2.0f * zNear * zFar) * fn;
    result.e[VAST_MAT4X4_AT(3, 2)] = -1.0f;

    return (result);
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_normalize(vast_vec3 a)
{
    f32 length_squared = a.x * a.x + a.y * a.y + a.z * a.z;
    f32 scalar = length_squared > 0.0f ? vast_invsqrtf(length_squared) : 0.0f;

    vast_vec3 result;

    result.x = a.x * scalar;
    result.y = a.y * scalar;
    result.z = a.z * scalar;

    return result;
}

VAST_API VAST_INLINE vast_mat4x4 vast_mat4x4_look_at(vast_vec3 eye, vast_vec3 target, vast_vec3 up)
{
    vast_vec3 f = vast_vec3_normalize(vast_vec3_sub(target, eye));
    vast_vec3 s = vast_vec3_normalize(vast_vec3_cross(f, up));
    vast_vec3 u = vast_vec3_normalize(vast_vec3_cross(s, f));

    vast_mat4x4 result;

    result.e[VAST_MAT4X4_AT(0, 0)] = s.x;
    result.e[VAST_MAT4X4_AT(1, 0)] = u.x;
    result.e[VAST_MAT4X4_AT(2, 0)] = -f.x;
    result.e[VAST_MAT4X4_AT(3, 0)] = 0.0f;

    result.e[VAST_MAT4X4_AT(0, 1)] = s.y;
    result.e[VAST_MAT4X4_AT(1, 1)] = u.y;
    result.e[VAST_MAT4X4_AT(2, 1)] = -f.y;
    result.e[VAST_MAT4X4_AT(3, 1)] = 0.0f;

    result.e[VAST_MAT4X4_AT(0, 2)] = s.z;
    result.e[VAST_MAT4X4_AT(1, 2)] = u.z;
    result.e[VAST_MAT4X4_AT(2, 2)] = -f.z;
    result.e[VAST_MAT4X4_AT(3, 2)] = 0.0f;

    result.e[VAST_MAT4X4_AT(0, 3)] = -vast_vec3_dot(s, eye);
    result.e[VAST_MAT4X4_AT(1, 3)] = -vast_vec3_dot(u, eye);
    result.e[VAST_MAT4X4_AT(2, 3)] = vast_vec3_dot(f, eye);
    result.e[VAST_MAT4X4_AT(3, 3)] = 1.0f;

    return result;
}

typedef struct vast_frustum
{
    vast_vec4 planes[6];
} vast_frustum;

typedef enum vast_frustum_plane
{
    VAST_FRUSTUM_LEFT = 0,
    VAST_FRUSTUM_RIGHT,
    VAST_FRUSTUM_BOTTOM,
    VAST_FRUSTUM_TOP,
    VAST_FRUSTUM_NEAR,
    VAST_FRUSTUM_FAR,
    VAST_FRUSTUM_COUNT

} vast_frustum_plane;

VAST_API vast_frustum vast_frustum_extract(vast_mat4x4 m)
{
    vast_frustum f = {0};

    u32 i;

    f.planes[VAST_FRUSTUM_LEFT].x = m.e[3] + m.e[0];
    f.planes[VAST_FRUSTUM_LEFT].y = m.e[7] + m.e[4];
    f.planes[VAST_FRUSTUM_LEFT].z = m.e[11] + m.e[8];
    f.planes[VAST_FRUSTUM_LEFT].w = m.e[15] + m.e[12];
    f.planes[VAST_FRUSTUM_RIGHT].x = m.e[3] - m.e[0];
    f.planes[VAST_FRUSTUM_RIGHT].y = m.e[7] - m.e[4];
    f.planes[VAST_FRUSTUM_RIGHT].z = m.e[11] - m.e[8];
    f.planes[VAST_FRUSTUM_RIGHT].w = m.e[15] - m.e[12];
    f.planes[VAST_FRUSTUM_BOTTOM].x = m.e[3] + m.e[1];
    f.planes[VAST_FRUSTUM_BOTTOM].y = m.e[7] + m.e[5];
    f.planes[VAST_FRUSTUM_BOTTOM].z = m.e[11] + m.e[9];
    f.planes[VAST_FRUSTUM_BOTTOM].w = m.e[15] + m.e[13];
    f.planes[VAST_FRUSTUM_TOP].x = m.e[3] - m.e[1];
    f.planes[VAST_FRUSTUM_TOP].y = m.e[7] - m.e[5];
    f.planes[VAST_FRUSTUM_TOP].z = m.e[11] - m.e[9];
    f.planes[VAST_FRUSTUM_TOP].w = m.e[15] - m.e[13];
    f.planes[VAST_FRUSTUM_NEAR].x = m.e[3] + m.e[2];
    f.planes[VAST_FRUSTUM_NEAR].y = m.e[7] + m.e[6];
    f.planes[VAST_FRUSTUM_NEAR].z = m.e[11] + m.e[10];
    f.planes[VAST_FRUSTUM_NEAR].w = m.e[15] + m.e[14];
    f.planes[VAST_FRUSTUM_FAR].x = m.e[3] - m.e[2];
    f.planes[VAST_FRUSTUM_FAR].y = m.e[7] - m.e[6];
    f.planes[VAST_FRUSTUM_FAR].z = m.e[11] - m.e[10];
    f.planes[VAST_FRUSTUM_FAR].w = m.e[15] - m.e[14];

    for (i = 0; i < 6; ++i)
    {
        vast_vec4 *p = &f.planes[i];

        f32 len = vast_sqrtf(
            p->x * p->x +
            p->y * p->y +
            p->z * p->z);

        if (len > 0.0f)
        {
            p->x /= len;
            p->y /= len;
            p->z /= len;
            p->w /= len;
        }
    }

    return f;
}

#endif /* VAST_MATH_LINEAR_ALGEBRA_H */