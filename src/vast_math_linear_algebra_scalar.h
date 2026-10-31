#ifndef VAST_MATH_LINEAR_ALGEBRA_SCALAR_H
#define VAST_MATH_LINEAR_ALGEBRA_SCALAR_H

#include "vast_math_basic.h"

/* #############################################################################
 * # [SECTION] Linear Algebra Math (Scalar)
 * #############################################################################
 */
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

#endif /* VAST_MATH_LINEAR_ALGEBRA_SCALAR_H */