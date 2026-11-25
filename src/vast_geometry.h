#ifndef VAST_GEOMETRY_H
#define VAST_GEOMETRY_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h" /* Basic types (u8, u32, ...)    */

/* #############################################################################
 * # [SECTION] Grid Generation with skirts
 * #############################################################################
 */
typedef struct vast_geometry_vertex_skirt
{
    f32 u;
    f32 v;
    f32 is_skirt;

} vast_geometry_vertex_skirt;

VAST_API i32 vast_geometry_grid_skirt(i32 n, vast_geometry_vertex_skirt *vertices_out, u16 *indices_out)
{
    i32 x, z;
    i32 v_idx = 0;
    i32 i_idx = 0;

    /* Core Vertices */
    for (z = 0; z < n; ++z)
    {
        for (x = 0; x < n; ++x)
        {
            vertices_out[v_idx].u = (f32)x / (f32)(n - 1);
            vertices_out[v_idx].v = (f32)z / (f32)(n - 1);
            vertices_out[v_idx].is_skirt = 0.0f;
            v_idx++;
        }
    }

    /*  Core Indices */
    for (z = 0; z < n - 1; ++z)
    {
        for (x = 0; x < n - 1; ++x)
        {
            u16 i0 = (u16)(z * n + x);
            u16 i1 = (u16)(z * n + (x + 1));
            u16 i2 = (u16)((z + 1) * n + x);
            u16 i3 = (u16)((z + 1) * n + (x + 1));

            indices_out[i_idx++] = i0;
            indices_out[i_idx++] = i2;
            indices_out[i_idx++] = i1;

            indices_out[i_idx++] = i1;
            indices_out[i_idx++] = i2;
            indices_out[i_idx++] = i3;
        }
    }

    /* Skirt Top Edge (z = 0) */
    for (x = 0; x < n - 1; ++x)
    {
        u16 p0 = (u16)(0 * n + x);
        u16 p1 = (u16)(0 * n + (x + 1));

        u16 s0 = (u16)v_idx++;
        u16 s1 = (u16)v_idx++;

        vertices_out[s0] = vertices_out[p0];
        vertices_out[s0].is_skirt = 1.0f;
        vertices_out[s1] = vertices_out[p1];
        vertices_out[s1].is_skirt = 1.0f;

        indices_out[i_idx++] = p0;
        indices_out[i_idx++] = p1;
        indices_out[i_idx++] = s0;

        indices_out[i_idx++] = p1;
        indices_out[i_idx++] = s1;
        indices_out[i_idx++] = s0;
    }

    /* Skirt Bottom Edge (z = n - 1) */
    for (x = 0; x < n - 1; ++x)
    {
        u16 p0 = (u16)((n - 1) * n + x);
        u16 p1 = (u16)((n - 1) * n + (x + 1));

        u16 s0 = (u16)v_idx++;
        u16 s1 = (u16)v_idx++;

        vertices_out[s0] = vertices_out[p0];
        vertices_out[s0].is_skirt = 1.0f;
        vertices_out[s1] = vertices_out[p1];
        vertices_out[s1].is_skirt = 1.0f;

        indices_out[i_idx++] = p0;
        indices_out[i_idx++] = s0;
        indices_out[i_idx++] = p1;

        indices_out[i_idx++] = p1;
        indices_out[i_idx++] = s0;
        indices_out[i_idx++] = s1;
    }

    /* Skirt Left Edge (x = 0) */
    for (z = 0; z < n - 1; ++z)
    {
        u16 p0 = (u16)(z * n + 0);
        u16 p1 = (u16)((z + 1) * n + 0);

        u16 s0 = (u16)v_idx++;
        u16 s1 = (u16)v_idx++;

        vertices_out[s0] = vertices_out[p0];
        vertices_out[s0].is_skirt = 1.0f;
        vertices_out[s1] = vertices_out[p1];
        vertices_out[s1].is_skirt = 1.0f;

        indices_out[i_idx++] = p0;
        indices_out[i_idx++] = s0;
        indices_out[i_idx++] = p1;

        indices_out[i_idx++] = p1;
        indices_out[i_idx++] = s0;
        indices_out[i_idx++] = s1;
    }

    /* Skirt Right Edge (x = n - 1) */
    for (z = 0; z < n - 1; ++z)
    {
        u16 p0 = (u16)(z * n + (n - 1));
        u16 p1 = (u16)((z + 1) * n + (n - 1));

        u16 s0 = (u16)v_idx++;
        u16 s1 = (u16)v_idx++;

        vertices_out[s0] = vertices_out[p0];
        vertices_out[s0].is_skirt = 1.0f;
        vertices_out[s1] = vertices_out[p1];
        vertices_out[s1].is_skirt = 1.0f;

        indices_out[i_idx++] = p0;
        indices_out[i_idx++] = p1;
        indices_out[i_idx++] = s0;

        indices_out[i_idx++] = p1;
        indices_out[i_idx++] = s1;
        indices_out[i_idx++] = s0;
    }

    return i_idx;
}

#endif /* VAST_GEOMETRY_H */