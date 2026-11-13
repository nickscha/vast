#ifndef VAST_MATH_LINEAR_ALGEBRA_H
#define VAST_MATH_LINEAR_ALGEBRA_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"      /* Basic types (u8, u32, ...)    */
#include "vast_math_basic.h" /* Basic math (sin, cos, sqrt)   */

/* #############################################################################
 * # [SECTION] Linear Algebra Math (Scalar)
 * #############################################################################
 */
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

typedef struct vast_vec3
{
    f32 x;
    f32 y;
    f32 z;

} vast_vec3;

static vast_vec3 vast_vec3_zero = {0.0f, 0.0f, 0.0f};

VAST_API VAST_INLINE vast_vec3 vast_vec3_init(f32 x, f32 y, f32 z)
{
    vast_vec3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_initf(f32 value)
{
    vast_vec3 result;

    result.x = value;
    result.y = value;
    result.z = value;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_add(vast_vec3 a, vast_vec3 b)
{
    vast_vec3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_sub(vast_vec3 a, vast_vec3 b)
{
    vast_vec3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_mul(vast_vec3 a, vast_vec3 b)
{
    vast_vec3 result;

    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_div(vast_vec3 a, vast_vec3 b)
{
    vast_vec3 result;

    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_addf(vast_vec3 a, f32 value)
{
    vast_vec3 result;

    result.x = a.x + value;
    result.y = a.y + value;
    result.z = a.z + value;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_subf(vast_vec3 a, f32 value)
{
    vast_vec3 result;

    result.x = a.x - value;
    result.y = a.y - value;
    result.z = a.z - value;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_mulf(vast_vec3 a, f32 value)
{
    vast_vec3 result;

    result.x = a.x * value;
    result.y = a.y * value;
    result.z = a.z * value;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_divf(vast_vec3 a, f32 value)
{
    vast_vec3 result;

    result.x = a.x / value;
    result.y = a.y / value;
    result.z = a.z / value;

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_abs(vast_vec3 a)
{
    vast_vec3 result;

    result.x = vast_absf(a.x);
    result.y = vast_absf(a.y);
    result.z = vast_absf(a.z);

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_minf(vast_vec3 a, f32 value)
{
    vast_vec3 result;

    result.x = vast_minf(a.x, value);
    result.y = vast_minf(a.y, value);
    result.z = vast_minf(a.z, value);

    return result;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_maxf(vast_vec3 a, f32 value)
{
    vast_vec3 result;

    result.x = vast_maxf(a.x, value);
    result.y = vast_maxf(a.y, value);
    result.z = vast_maxf(a.z, value);

    return result;
}

VAST_API VAST_INLINE f32 vast_vec3_length(vast_vec3 a)
{
    return vast_sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

VAST_API VAST_INLINE f32 vast_vec3_dot(vast_vec3 a, vast_vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

VAST_API VAST_INLINE vast_vec3 vast_vec3_cross(vast_vec3 a, vast_vec3 b)
{
    vast_vec3 result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
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

#define VAST_MAT2X2_ELEMENT_COUNT 4

#ifdef VAST_MAT_ROW_MAJOR_ORDER
#define VAST_MAT2X2_AT(row, col) ((row) * 2 + (col)) /* Row-major order */
#else
#define VAST_MAT2X2_AT(row, col) ((col) * 2 + (row)) /* Column-major order */
#endif

typedef struct vast_mat2x2
{
    f32 e[VAST_MAT2X2_ELEMENT_COUNT];

} vast_mat2x2;

VAST_API VAST_INLINE vast_mat2x2 vast_mat2x2_init(f32 m00, f32 m01, f32 m10, f32 m11)
{
    vast_mat2x2 result;

    result.e[VAST_MAT2X2_AT(0, 0)] = m00;
    result.e[VAST_MAT2X2_AT(0, 1)] = m01;
    result.e[VAST_MAT2X2_AT(1, 0)] = m10;
    result.e[VAST_MAT2X2_AT(1, 1)] = m11;

    return result;
}

VAST_API VAST_INLINE vast_mat2x2 vast_mat2x2_rot2d(f32 angle)
{
    f32 s = vast_sinf(angle);
    f32 c = vast_cosf(angle);

    return vast_mat2x2_init(c, -s, s, c);
}

VAST_API VAST_INLINE void vast_vec2_mul_mat2x2(f32 *a, f32 *b, vast_mat2x2 m)
{
    f32 x = *a;
    f32 y = *b;

    *a = x * m.e[VAST_MAT2X2_AT(0, 0)] + y * m.e[VAST_MAT2X2_AT(1, 0)];
    *b = x * m.e[VAST_MAT2X2_AT(0, 1)] + y * m.e[VAST_MAT2X2_AT(1, 1)];
}

#define VAST_MAT4X4_ELEMENT_COUNT 16

#ifdef VAST_MAT_ROW_MAJOR_ORDER
#define VAST_MAT4X4_AT(row, col) ((row) * 4 + (col)) /* Row-major order */
#else
#define VAST_MAT4X4_AT(row, col) ((col) * 4 + (row)) /* Column-major order */
#endif

typedef struct vast_mat4x4
{
    f32 e[VAST_MAT4X4_ELEMENT_COUNT];
} vast_mat4x4;

static vast_mat4x4 vast_mat4x4_zero =
    {{0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f}};

VAST_API VAST_INLINE vast_mat4x4 vast_mat4x4_mul(vast_mat4x4 a, vast_mat4x4 b)
{
    vast_mat4x4 result;

    i32 i;
    i32 j;

    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            result.e[VAST_MAT4X4_AT(i, j)] =
                a.e[VAST_MAT4X4_AT(i, 0)] * b.e[VAST_MAT4X4_AT(0, j)] +
                a.e[VAST_MAT4X4_AT(i, 1)] * b.e[VAST_MAT4X4_AT(1, j)] +
                a.e[VAST_MAT4X4_AT(i, 2)] * b.e[VAST_MAT4X4_AT(2, j)] +
                a.e[VAST_MAT4X4_AT(i, 3)] * b.e[VAST_MAT4X4_AT(3, j)];
        }
    }

    return result;
}

VAST_API VAST_INLINE vast_mat4x4 vast_mat4x4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    f32 width = right - left;
    f32 height = top - bottom;
    f32 depth = far - near;

    vast_mat4x4 result = vast_mat4x4_zero;

    result.e[VAST_MAT4X4_AT(0, 0)] = 2.0f / width;
    result.e[VAST_MAT4X4_AT(0, 3)] = -(right + left) / width;
    result.e[VAST_MAT4X4_AT(1, 1)] = 2.0f / height;
    result.e[VAST_MAT4X4_AT(1, 3)] = -(top + bottom) / height;
    result.e[VAST_MAT4X4_AT(2, 2)] = -2.0f / depth;
    result.e[VAST_MAT4X4_AT(2, 3)] = -(far + near) / depth;
    result.e[VAST_MAT4X4_AT(3, 3)] = 1.0f;

    return result;
}

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

typedef struct vast_vec4
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;

} vast_vec4;

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

typedef struct vast_frustum
{
    vast_vec4 planes[VAST_FRUSTUM_COUNT];

} vast_frustum;

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