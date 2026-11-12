#ifndef VAST_TILES_H
#define VAST_TILES_H

#include "vast_types.h"

/* #############################################################################
 * # [SECTION] Tiling Logic
 * #############################################################################
 */
#ifndef VAST_TILES_PER_SIDE
#define VAST_TILES_PER_SIDE 33
#endif
#define VAST_TILES_HALF VAST_TILES_PER_SIDE / 2;
#define VAST_TILES_TOTAL (VAST_TILES_PER_SIDE * VAST_TILES_PER_SIDE)
#define VAST_TILES_MAX_LOD_LEVELS 6 /* Not more than this */

/* Edge Mask */
#define VAST_EDGE_NONE 0
#define VAST_EDGE_NORTH 1 << 0
#define VAST_EDGE_SOUTH 1 << 1
#define VAST_EDGE_WEST 1 << 2
#define VAST_EDGE_EAST 1 << 3
#define VAST_EDGE_COMBINATIONS 16

/* SoA tiles setup */
typedef struct vast_tiles
{
    /* General information */
    i32 origin_x; /* The bottom left origin X */
    i32 origin_z; /* The bottom left origin Z */
    i32 camera_x;
    i32 camera_z;

    /* LOD stats (what is the maximum lod and how many tiles per LOD) */
    i32 tile_lod_max;
    i32 tile_per_lod_count[VAST_TILES_MAX_LOD_LEVELS];

    /* Data Arrays (Indexed via toroidal wrapping) */
    i32 tile_x[VAST_TILES_TOTAL];
    i32 tile_z[VAST_TILES_TOTAL];
    i8 tile_lod_required[VAST_TILES_TOTAL]; /* The LOD required after update */
    i8 tile_lod_current[VAST_TILES_TOTAL];  /* The current LOD used (has to be updated by application) */

    /* Sparse Dirty flag tracking */
    u16 dirty_indices[VAST_TILES_TOTAL]; /* Which index ot the tile_x/z is marked dirty */
    u16 dirty_indices_count;             /* Total number of dirty tile indices */

} vast_tiles;

VAST_API VAST_INLINE i32 vast_modi(i32 a, i32 b)
{
    i32 r = a % b;
    return r < 0 ? r + b : r;
}

VAST_API VAST_INLINE i32 vast_absi(i32 v)
{
    return v < 0 ? -v : v;
}

VAST_API VAST_INLINE i32 vast_maxi(i32 a, i32 b)
{
    return a > b ? a : b;
}

VAST_API VAST_INLINE u32 vast_tile_index(i32 x, i32 z)
{
    i32 slot_x = vast_modi(x, VAST_TILES_PER_SIDE);
    i32 slot_z = vast_modi(z, VAST_TILES_PER_SIDE);
    return (u32)(slot_z * VAST_TILES_PER_SIDE + slot_x);
}

VAST_API VAST_INLINE i8 vast_tile_lod(i32 x, i32 z, i32 center_x, i32 center_z)
{
    i32 dx = vast_absi(x - center_x);
    i32 dz = vast_absi(z - center_z);
    i32 distance = vast_maxi(dx, dz);

    /* TODO(nickscha): apply better distance distribution */
    i8 lod = (i8)(distance / 2);

    if (lod >= VAST_TILES_MAX_LOD_LEVELS)
    {
        lod = VAST_TILES_MAX_LOD_LEVELS - 1;
    }

    return lod;
}

VAST_API VAST_INLINE u8 vast_tile_is_on_lod_edge(vast_tiles *t, u32 tile_idx)
{
    i32 current_lod = t->tile_lod_current[tile_idx];
    i32 x, z;
    u32 n_idx, e_idx, s_idx, w_idx;

    if (current_lod < 0)
    {
        return 0;
    }

    x = t->tile_x[tile_idx];
    z = t->tile_z[tile_idx];

    /* Get neighbors */
    n_idx = vast_tile_index(x, z - 1);
    e_idx = vast_tile_index(x + 1, z);
    s_idx = vast_tile_index(x, z + 1);
    w_idx = vast_tile_index(x - 1, z);

    /* Does a neighbor has a different lod? */
    if ((t->tile_x[n_idx] == x && t->tile_z[n_idx] == z - 1 && t->tile_lod_current[n_idx] > current_lod) ||
        (t->tile_x[e_idx] == x + 1 && t->tile_z[e_idx] == z && t->tile_lod_current[e_idx] > current_lod) ||
        (t->tile_x[s_idx] == x && t->tile_z[s_idx] == z + 1 && t->tile_lod_current[s_idx] > current_lod) ||
        (t->tile_x[w_idx] == x - 1 && t->tile_z[w_idx] == z && t->tile_lod_current[w_idx] > current_lod))
    {
        return 1;
    }

    return 0;
}

VAST_API VAST_INLINE u8 vast_tile_get_lod_edge_mask(vast_tiles *t, u32 tile_idx)
{
    i32 current_lod = t->tile_lod_current[tile_idx];
    i32 x, z;
    u8 mask;

    if (current_lod < 0)
    {
        return VAST_EDGE_NONE;
    }

    x = t->tile_x[tile_idx];
    z = t->tile_z[tile_idx];
    mask = VAST_EDGE_NONE;

    /* North neighbor (z - 1) */
    {
        u32 n_idx = vast_tile_index(x, z - 1);

        if (t->tile_x[n_idx] == x && t->tile_z[n_idx] == z - 1)
        {
            i32 n_lod = t->tile_lod_current[n_idx];

            if (n_lod > current_lod)
            {
                mask |= VAST_EDGE_NORTH;
            }
        }
    }

    /* East neighbor (x + 1) */
    {
        u32 e_idx = vast_tile_index(x + 1, z);

        if (t->tile_x[e_idx] == x + 1 && t->tile_z[e_idx] == z)
        {
            i32 e_lod = t->tile_lod_current[e_idx];

            if (e_lod > current_lod)
            {
                mask |= VAST_EDGE_EAST;
            }
        }
    }

    /* South neighbor (z + 1) */
    {
        u32 s_idx = vast_tile_index(x, z + 1);

        if (t->tile_x[s_idx] == x && t->tile_z[s_idx] == z + 1)
        {
            i32 s_lod = t->tile_lod_current[s_idx];

            if (s_lod > current_lod)
            {
                mask |= VAST_EDGE_SOUTH;
            }
        }
    }

    /* West neighbor (x - 1) */
    {
        u32 w_idx = vast_tile_index(x - 1, z);

        if (t->tile_x[w_idx] == x - 1 && t->tile_z[w_idx] == z)
        {
            i32 w_lod = t->tile_lod_current[w_idx];

            if (w_lod > current_lod)
            {
                mask |= VAST_EDGE_WEST;
            }
        }
    }

    return mask;
}

VAST_API VAST_INLINE u8 vast_tile_is_dirty(vast_tiles *t, u32 tile_index)
{
    u16 i;

    for (i = 0; i < t->dirty_indices_count; ++i)
    {
        if (t->dirty_indices[i] == tile_index)
        {
            return 1;
        }
    }

    return 0;
}

VAST_API VAST_INLINE void vast_tiles_sort_dirty_by_distance(vast_tiles *t)
{
    i32 n = (i32)t->dirty_indices_count;
    i32 gap, i, j;

    if (n < 2)
    {
        return;
    }

    for (gap = n / 2; gap > 0; gap /= 2)
    {
        for (i = gap; i < n; ++i)
        {
            u16 temp_idx = t->dirty_indices[i];
            i32 temp_dist = t->tile_lod_required[temp_idx];

            for (j = i; j >= gap && t->tile_lod_required[t->dirty_indices[j - gap]] < temp_dist; j -= gap)
            {
                t->dirty_indices[j] = t->dirty_indices[j - gap];
            }
            t->dirty_indices[j] = temp_idx;
        }
    }
}

VAST_API VAST_INLINE void vast_tiles_init(vast_tiles *t, i32 camera_tile_x, i32 camera_tile_z)
{
    i32 x, z;

    t->origin_x = camera_tile_x - VAST_TILES_HALF;
    t->origin_z = camera_tile_z - VAST_TILES_HALF;
    t->camera_x = camera_tile_x - 424242; /* force update */
    t->camera_z = camera_tile_z - 424242; /* force update  */
    t->dirty_indices_count = 0;

    /* Mark all tiles dirty and calculate lod distance */
    for (z = t->origin_z; z < t->origin_z + VAST_TILES_PER_SIDE; ++z)
    {
        for (x = t->origin_x; x < t->origin_x + VAST_TILES_PER_SIDE; ++x)
        {
            u32 i = vast_tile_index(x, z);
            i8 lod = vast_tile_lod(x, z, camera_tile_x, camera_tile_z);

            t->tile_x[i] = x;
            t->tile_z[i] = z;
            t->tile_lod_required[i] = lod;
            t->tile_lod_current[i] = -1; /* -1 = Uninitialized/Unknown */
            t->dirty_indices[t->dirty_indices_count++] = (u16)i;
            t->tile_per_lod_count[(i32)lod]++;

            if (lod > t->tile_lod_max)
            {
                t->tile_lod_max = lod;
            }
        }
    }

    /* TODO(nickscha): Check that tile_lod_max is less than VAST_TILES_MAX_LOD_LEVELS */
    vast_tiles_sort_dirty_by_distance(t);
}

/* Toroidal wrap around */
VAST_API VAST_INLINE void vast_tiles_update(vast_tiles *t, i32 camera_tile_x, i32 camera_tile_z)
{
    i32 x, z;
    u32 i;
    i32 new_origin_x;
    i32 new_origin_z;

    new_origin_x = camera_tile_x - VAST_TILES_HALF;
    new_origin_z = camera_tile_z - VAST_TILES_HALF;

    /* If camera position hasn't changed there is nothing to calculate */
    if (t->camera_x == camera_tile_x && t->camera_z == camera_tile_z)
    {
        return;
    }

    t->camera_x = camera_tile_x;
    t->camera_z = camera_tile_z;

    /* If camera is to far away reinitialize everyting */
    if (vast_absi(new_origin_x - t->origin_x) >= VAST_TILES_PER_SIDE ||
        vast_absi(new_origin_z - t->origin_z) >= VAST_TILES_PER_SIDE)
    {
        vast_tiles_init(t, camera_tile_x, camera_tile_z);
        return;
    }

    /* Move East */
    while (t->origin_x < new_origin_x)
    {
        i32 target_x = t->origin_x + VAST_TILES_PER_SIDE;

        for (z = t->origin_z; z < t->origin_z + VAST_TILES_PER_SIDE; ++z)
        {
            u32 i = vast_tile_index(target_x, z);

            t->tile_x[i] = target_x;
            t->tile_z[i] = z;
            t->dirty_indices[t->dirty_indices_count++] = (u16)i;
        }

        t->origin_x++;
    }

    /* Move West */
    while (t->origin_x > new_origin_x)
    {
        i32 target_x;

        t->origin_x--;
        target_x = t->origin_x;

        for (z = t->origin_z; z < t->origin_z + VAST_TILES_PER_SIDE; ++z)
        {
            u32 i = vast_tile_index(target_x, z);

            t->tile_x[i] = target_x;
            t->tile_z[i] = z;
            t->dirty_indices[t->dirty_indices_count++] = (u16)i;
        }
    }

    /* Move South */
    while (t->origin_z < new_origin_z)
    {
        i32 target_z = t->origin_z + VAST_TILES_PER_SIDE;

        for (x = t->origin_x; x < t->origin_x + VAST_TILES_PER_SIDE; ++x)
        {
            u32 i = vast_tile_index(x, target_z);

            t->tile_x[i] = x;
            t->tile_z[i] = target_z;
            t->dirty_indices[t->dirty_indices_count++] = (u16)i;
        }

        t->origin_z++;
    }

    /* Move North */
    while (t->origin_z > new_origin_z)
    {
        i32 target_z;

        t->origin_z--;
        target_z = t->origin_z;

        for (x = t->origin_x; x < t->origin_x + VAST_TILES_PER_SIDE; ++x)
        {
            u32 i = vast_tile_index(x, target_z);

            t->tile_x[i] = x;
            t->tile_z[i] = target_z;
            t->dirty_indices[t->dirty_indices_count++] = (u16)i;
        }
    }

    /* Distance Calculation */
    for (i = 0; i < VAST_TILES_TOTAL; ++i)
    {
        i32 distance_old = t->tile_lod_required[i];
        i8 distance_new = vast_tile_lod(t->tile_x[i], t->tile_z[i], camera_tile_x, camera_tile_z);

        if (distance_old != distance_new)
        {
            t->tile_lod_required[i] = distance_new;

            if (!vast_tile_is_dirty(t, i))
            {
                t->dirty_indices[t->dirty_indices_count++] = (u16)i;
            }
        }
    }

    vast_tiles_sort_dirty_by_distance(t);
}

#endif /* VAST_TILES_H */