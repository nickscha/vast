#ifndef VAST_GEOMETRY_H
#define VAST_GEOMETRY_H

#include "vast_types.h"

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

    /* 1. Core Vertices */
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

    /* 2. Core Indices */
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

    /* 3. Skirt Top Edge (z = 0) - Outward normal points -Z */
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

    /* 4. Skirt Bottom Edge (z = n - 1) - Outward normal points +Z */
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

    /* 5. Skirt Left Edge (x = 0) - Outward normal points -X */
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

    /* 6. Skirt Right Edge (x = n - 1) - Outward normal points +X */
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

/* #############################################################################
 * # [SECTION] LOD Grid Generation with degen. indices edge masks
 * #############################################################################
 */
typedef enum vast_geometry_lod_edge
{
    VAST_GEOMETRY_LOD_EDGE_NONE = 0,
    VAST_GEOMETRY_LOD_EDGE_NORTH = 1 << 0, /* -Z boundary (z = 0)                      */
    VAST_GEOMETRY_LOD_EDGE_SOUTH = 1 << 1, /* +Z boundary (z = grid_resolution - 2)    */
    VAST_GEOMETRY_LOD_EDGE_WEST = 1 << 2,  /* -X boundary (x = 0)                      */
    VAST_GEOMETRY_LOD_EDGE_EAST = 1 << 3   /* +X boundary (x = grid_resolution - 2)    */

} vast_geometry_lod_edge;

/* Creates a fixed unit grid for the specified edge flags.
   When edge flags are specified the borders of the grid create on the edges degenerate indices at half
   resolution for allowing seamless stiching of differen LOD grids.

   Usage:

   static u32 grid_vao;
   static i32 gridIndexCounts[LOD_COUNT][VAST_EDGE_COMBINATIONS] = {0};
   static u32 grid_ibos[LOD_COUNT][VAST_EDGE_COMBINATIONS] = {0};

   u8 mask;
   u32 i;

   u16 gridIndices[VAST_TILE_MAX_INDICES];

   glGenVertexArrays(1, &grid_vao);
   glBindVertexArray(grid_vao);

   for (i = 0; i < (u32)t.tile_lod_max + 1; ++i)
   {
       i32 grid_resolution = ((VAST_TILE_GRID_SIZE - 1) / (1 << (u32)i)) + 1;

       for (mask = 0; mask < VAST_EDGE_COMBINATIONS; ++mask)
       {
           gridIndexCounts[i][mask] = vast_geometry_grid_lod(grid_resolution, (u32)mask, gridIndices);

           glGenBuffers(1, &grid_ibos[i][mask]);
           glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ibos[i][mask]);
           glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndexCounts[i][mask] * (i32)sizeof(u16), gridIndices, GL_STATIC_DRAW);
       }
   }
*/
VAST_API VAST_INLINE i32 vast_geometry_grid_lod(i32 grid_resolution, u32 edge_flags, u16 *grid_indices)
{
    i32 x, z;
    i32 i = 0;
    i32 max_idx = grid_resolution - 1;

    for (z = 0; z < max_idx; ++z)
    {
        for (x = 0; x < max_idx; ++x)
        {
            u16 i0 = (u16)(z * grid_resolution + x);
            u16 i1 = (u16)(i0 + 1);
            u16 i2 = (u16)(i0 + grid_resolution);
            u16 i3 = (u16)(i2 + 1);

            i32 is_north = (z == 0) && (edge_flags & VAST_GEOMETRY_LOD_EDGE_NORTH);
            i32 is_south = (z == max_idx - 1) && (edge_flags & VAST_GEOMETRY_LOD_EDGE_SOUTH);
            i32 is_west = (x == 0) && (edge_flags & VAST_GEOMETRY_LOD_EDGE_WEST);
            i32 is_east = (x == max_idx - 1) && (edge_flags & VAST_GEOMETRY_LOD_EDGE_EAST);

            /* SOUTH-WEST CORNER (x = 0, z = max_idx - 1) */
            if (is_south && is_west)
            {
                u16 i_sw_outer = (u16)(max_idx * grid_resolution);
                u16 i_w_top = (u16)((max_idx - 2) * grid_resolution);
                u16 i_s_right = (u16)(max_idx * grid_resolution + 2);

                /* Fan from inner anchor i1 to fill the dual-stitched corner */
                grid_indices[i++] = i1;
                grid_indices[i++] = i_w_top;
                grid_indices[i++] = i_sw_outer;

                grid_indices[i++] = i1;
                grid_indices[i++] = i_sw_outer;
                grid_indices[i++] = i_s_right;
                continue;
            }

            /* SOUTH-EAST CORNER (x = max_idx - 1, z = max_idx - 1) */
            if (is_south && is_east)
            {
                u16 i_se_outer = (u16)(max_idx * grid_resolution + max_idx);
                u16 i_e_top = (u16)((max_idx - 2) * grid_resolution + max_idx);
                u16 i_s_left = (u16)(max_idx * grid_resolution + max_idx - 2);

                grid_indices[i++] = i0;
                grid_indices[i++] = i_s_left;
                grid_indices[i++] = i_se_outer;

                grid_indices[i++] = i0;
                grid_indices[i++] = i_se_outer;
                grid_indices[i++] = i_e_top;
                continue;
            }

            /* NORTH-WEST CORNER (x = 0, z = 0) */
            if (is_north && is_west)
            {
                u16 i_nw_outer = 0;
                u16 i_n_right = 2;
                u16 i_w_bottom = (u16)(2 * grid_resolution);

                grid_indices[i++] = i3;
                grid_indices[i++] = i_nw_outer;
                grid_indices[i++] = i_w_bottom;

                grid_indices[i++] = i3;
                grid_indices[i++] = i_n_right;
                grid_indices[i++] = i_nw_outer;
                continue;
            }

            /* NORTH-EAST CORNER (x = max_idx - 1, z = 0) */
            if (is_north && is_east)
            {
                u16 i_ne_outer = (u16)max_idx;
                u16 i_n_left = (u16)(max_idx - 2);
                u16 i_e_bottom = (u16)(2 * grid_resolution + max_idx);

                grid_indices[i++] = i2;
                grid_indices[i++] = i_ne_outer;
                grid_indices[i++] = i_n_left;

                grid_indices[i++] = i2;
                grid_indices[i++] = i_e_bottom;
                grid_indices[i++] = i_ne_outer;
                continue;
            }

            /* NORTH EDGE (-Z) */
            if (is_north)
            {
                u16 i_outer_left = (u16)(z * grid_resolution + (x - (x % 2)));
                u16 i_outer_right = (u16)(i_outer_left + 2);

                if ((x % 2) == 0)
                {
                    grid_indices[i++] = i_outer_left;
                    grid_indices[i++] = i2;
                    grid_indices[i++] = i3;
                }
                else
                {
                    grid_indices[i++] = i_outer_left;
                    grid_indices[i++] = i2;
                    grid_indices[i++] = i_outer_right;

                    grid_indices[i++] = i_outer_right;
                    grid_indices[i++] = i2;
                    grid_indices[i++] = i3;
                }
                continue;
            }

            /* SOUTH EDGE (+Z) */
            if (is_south)
            {
                u16 i_outer_left = (u16)((z + 1) * grid_resolution + (x - (x % 2)));
                u16 i_outer_right = (u16)(i_outer_left + 2);

                if ((x % 2) == 0)
                {
                    grid_indices[i++] = i0;
                    grid_indices[i++] = i_outer_left;
                    grid_indices[i++] = i1;

                    grid_indices[i++] = i1;
                    grid_indices[i++] = i_outer_left;
                    grid_indices[i++] = i_outer_right;
                }
                else
                {
                    grid_indices[i++] = i0;
                    grid_indices[i++] = i_outer_right;
                    grid_indices[i++] = i1;
                }
                continue;
            }

            /* WEST EDGE (-X) */
            if (is_west)
            {
                u16 i_outer_top = (u16)((z - (z % 2)) * grid_resolution + x);
                u16 i_outer_bottom = (u16)(i_outer_top + (2 * grid_resolution));

                if ((z % 2) == 0)
                {
                    grid_indices[i++] = i_outer_top;
                    grid_indices[i++] = i3;
                    grid_indices[i++] = i1;
                }
                else
                {
                    grid_indices[i++] = i_outer_top;
                    grid_indices[i++] = i_outer_bottom;
                    grid_indices[i++] = i3;

                    grid_indices[i++] = i1;
                    grid_indices[i++] = i_outer_top;
                    grid_indices[i++] = i3;
                }
                continue;
            }

            /* EAST EDGE (+X) */
            if (is_east)
            {
                u16 i_outer_top = (u16)((z - (z % 2)) * grid_resolution + (x + 1));
                u16 i_outer_bottom = (u16)(i_outer_top + (2 * grid_resolution));

                if ((z % 2) == 0)
                {
                    grid_indices[i++] = i0;
                    grid_indices[i++] = i2;
                    grid_indices[i++] = i_outer_top;

                    grid_indices[i++] = i_outer_top;
                    grid_indices[i++] = i2;
                    grid_indices[i++] = i_outer_bottom;
                }
                else
                {
                    grid_indices[i++] = i0;
                    grid_indices[i++] = i2;
                    grid_indices[i++] = i_outer_bottom;
                }
                continue;
            }

            /* Interior Quad */
            grid_indices[i++] = i0;
            grid_indices[i++] = i2;
            grid_indices[i++] = i1;

            grid_indices[i++] = i1;
            grid_indices[i++] = i2;
            grid_indices[i++] = i3;
        }
    }

    return i;
}

#endif /* VAST_GEOMETRY_H */