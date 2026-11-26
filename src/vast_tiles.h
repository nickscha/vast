#ifndef VAST_TILES_H
#define VAST_TILES_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)           */
#include "vast_math_basic.h"          /* Basic math for world space functions */
#include "vast_math_linear_algebra.h" /* Vec4 for tile bounds calculation     */

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

/* SoA tiles setup */
typedef struct vast_tiles
{
    /* General information */
    i32 origin_x; /* The bottom left origin X */
    i32 origin_z; /* The bottom left origin Z */
    i32 camera_x;
    i32 camera_z;

    f32 world_size; /* How much area does one tile cover in world space (has to be power of two) */

    /* LOD stats (what is the maximum lod and how many tiles per LOD) */
    i32 tile_lod_max;
    i32 tile_per_lod_count[VAST_TILES_MAX_LOD_LEVELS];

    /* Data Arrays (Indexed via toroidal wrapping) */
    i32 tile_x[VAST_TILES_TOTAL];
    i32 tile_z[VAST_TILES_TOTAL];
    i8 tile_lod_required[VAST_TILES_TOTAL];           /* The LOD required after update */
    i8 tile_lod_current[VAST_TILES_TOTAL];            /* The current LOD used (has to be updated by application) */
    f32 tile_lod_transition_factor[VAST_TILES_TOTAL]; /* TODO(nickscha): LOD Geomorphing factor */
    i32 texture_layer[VAST_TILES_TOTAL];              /* TODO(nickscha): OpenGL state, shouldnt be here but currently easier to manage */

    /* Sparse Dirty flag tracking */
    u16 dirty_indices[VAST_TILES_TOTAL]; /* Which index ot the tile_x/z is marked dirty */
    u16 dirty_indices_count;             /* Total number of dirty tile indices */

} vast_tiles;

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

VAST_API VAST_INLINE u8 vast_tile_is_on_lod_outer_edge(vast_tiles *t, u32 tile_idx)
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

VAST_API VAST_INLINE u32 vast_tile_calc_pool_size(vast_tiles *t, u32 lod)
{
    u32 steady;
    u32 lod_count;
    u32 headroom;
    u32 total;

    lod_count = (u32)t->tile_lod_max + 1;

    if (lod_count < 1)
    {
        lod_count = 1;
    }

    if (lod >= lod_count)
    {
        lod = lod_count - 1;
    }

    steady = (u32)t->tile_per_lod_count[lod];

    /* Scale headroom by grid size and coarse-LOD ring size */
    headroom = (VAST_TILES_PER_SIDE / 4) * (lod + 1) * 2 + 1;

    /* Keep finer LODs tighter on large pools while giving outer LODs extra margin */
    if (lod == 0 && headroom > 7)
    {
        headroom = 7;
    }

    total = steady + headroom;

    if (total > VAST_TILES_TOTAL)
    {
        total = VAST_TILES_TOTAL;
    }

    if (total < 1)
    {
        total = 1;
    }

    return total;
}

VAST_API VAST_INLINE u8 vast_tile_is_dirty(vast_tiles *t, u32 tile_index)
{
    u16 dirty_index;

    for (dirty_index = 0; dirty_index < t->dirty_indices_count; ++dirty_index)
    {
        if (t->dirty_indices[dirty_index] == tile_index)
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

/* #############################################################################
 * # [SECTION] World Space specific helper functions
 * #############################################################################
 */
VAST_API VAST_INLINE i32 vast_tile_calculate_camera_tile(vast_tiles *t, f32 camera_position)
{
    return (i32)vast_floorf((camera_position + t->world_size * 0.5f) / t->world_size);
}

VAST_API VAST_INLINE vast_vec4 vast_tile_calculate_bounds(vast_tiles *t, f32 position_x, f32 position_z)
{
    f32 world_size_half = t->world_size * 0.5f;

    i32 camera_tile_x = vast_tile_calculate_camera_tile(t, position_x);
    i32 camera_tile_z = vast_tile_calculate_camera_tile(t, position_z);

    f32 tile_x_min = (f32)camera_tile_x * t->world_size - world_size_half;
    f32 tile_x_max = (f32)tile_x_min + t->world_size;
    f32 tile_z_min = (f32)camera_tile_z * t->world_size - world_size_half;
    f32 tile_z_max = (f32)tile_z_min + t->world_size;

    return vast_vec4_init(tile_x_min, tile_x_max, tile_z_min, tile_z_max);
}

VAST_API VAST_INLINE f32 vast_tile_calculate_camera_backface_edge_factor(
    vast_tiles *t,
    f32 camera_position_x,
    f32 camera_position_z,
    f32 forward_x,
    f32 forward_z,
    f32 threshold)
{
    f32 distance;
    f32 threshold_distance;
    f32 dx;
    f32 dz;

    vast_vec4 tile_bounds = vast_tile_calculate_bounds(t, camera_position_x, camera_position_z);

    f32 min_x = tile_bounds.x;
    f32 max_x = tile_bounds.y;
    f32 min_z = tile_bounds.z;
    f32 max_z = tile_bounds.w;

    threshold_distance = (max_x - min_x) * threshold;

    if ((max_z - min_z) * threshold > threshold_distance)
    {
        threshold_distance = (max_z - min_z) * threshold;
    }

    if (threshold_distance <= 0.0f)
    {
        return 1.0f;
    }

    distance = 1.0e30f;

    /* Opposite X direction. */
    if (forward_x > 0.0f)
    {
        dx = camera_position_x - min_x;

        if (dx < distance)
        {
            distance = dx;
        }
    }
    else if (forward_x < 0.0f)
    {
        dx = max_x - camera_position_x;

        if (dx < distance)
        {
            distance = dx;
        }
    }

    /* Opposite Z direction. */
    if (forward_z > 0.0f)
    {
        dz = camera_position_z - min_z;

        if (dz < distance)
        {
            distance = dz;
        }
    }
    else if (forward_z < 0.0f)
    {
        dz = max_z - camera_position_z;

        if (dz < distance)
        {
            distance = dz;
        }
    }

    if (distance == 1.0e30f)
    {
        return 1.0f;
    }

    distance /= threshold_distance;

    return vast_clampf(distance, 0.0f, 1.0f);
}

VAST_API VAST_INLINE u8 vast_tile_is_visible(
    vast_tiles *t,
    vast_frustum *f,
    u32 tile_index,
    f32 minHeight,
    f32 maxHeight)
{
    f32 tile_x = (f32)t->tile_x[tile_index];
    f32 tile_z = (f32)t->tile_z[tile_index];

    f32 world_size_half = t->world_size * 0.5f;

    f32 minX = tile_x * t->world_size - world_size_half;
    f32 maxX = tile_x * t->world_size + world_size_half;

    f32 minZ = tile_z * t->world_size - world_size_half;
    f32 maxZ = tile_z * t->world_size + world_size_half;

    i32 plane_index;

    for (plane_index = 0; plane_index < VAST_FRUSTUM_COUNT; ++plane_index)
    {
        vast_vec4 *plane = &f->planes[plane_index];

        f32 px = (plane->x >= 0.0f) ? maxX : minX;
        f32 py = (plane->y >= 0.0f) ? maxHeight : minHeight;
        f32 pz = (plane->z >= 0.0f) ? maxZ : minZ;

        if (plane->x * px + plane->y * py + plane->z * pz + plane->w < 0.0f)
        {
            return 0;
        }
    }

    return 1;
}

/* #############################################################################
 * # [SECTION] User API calls
 * #############################################################################
 */
VAST_API VAST_INLINE void vast_tiles_init(vast_tiles *t, f32 camera_position_x, f32 camera_position_z, f32 world_size)
{
    i32 x, z, lod;

    i32 camera_tile_x;
    i32 camera_tile_z;

    t->world_size = world_size;
    t->dirty_indices_count = 0;

    camera_tile_x = vast_tile_calculate_camera_tile(t, camera_position_x);
    camera_tile_z = vast_tile_calculate_camera_tile(t, camera_position_z);

    t->origin_x = camera_tile_x - VAST_TILES_HALF;
    t->origin_z = camera_tile_z - VAST_TILES_HALF;
    t->camera_x = camera_tile_x - 424242; /* force update */
    t->camera_z = camera_tile_z - 424242; /* force update  */

    /* Rest tile lod counter */
    t->tile_lod_max = 0;

    for (lod = 0; lod < VAST_TILES_MAX_LOD_LEVELS; ++lod)
    {
        t->tile_per_lod_count[lod] = 0;
    }

    /* Mark all tiles dirty and calculate lod distance */
    for (z = t->origin_z; z < t->origin_z + VAST_TILES_PER_SIDE; ++z)
    {
        for (x = t->origin_x; x < t->origin_x + VAST_TILES_PER_SIDE; ++x)
        {
            u32 tile_index = vast_tile_index(x, z);
            i8 tile_lod = vast_tile_lod(x, z, camera_tile_x, camera_tile_z);

            t->tile_x[tile_index] = x;
            t->tile_z[tile_index] = z;
            t->tile_lod_required[tile_index] = tile_lod;
            t->tile_lod_current[tile_index] = -1;             /* -1 = Uninitialized/Unknown */
            t->tile_lod_transition_factor[tile_index] = 1.0f; /* Set transition to default 1.0f (unchanged) */
            t->texture_layer[tile_index] = -1;                /* -1 = Uninitialized/Unknown */
            t->dirty_indices[t->dirty_indices_count++] = (u16)tile_index;
            t->tile_per_lod_count[(i32)tile_lod]++;

            if (tile_lod > t->tile_lod_max)
            {
                t->tile_lod_max = tile_lod;
            }
        }
    }

    /* TODO(nickscha): Check that tile_lod_max is less than VAST_TILES_MAX_LOD_LEVELS */
    vast_tiles_sort_dirty_by_distance(t);
}

/* Toroidal wrap around */
VAST_API VAST_INLINE void vast_tiles_update(vast_tiles *t, f32 camera_position_x, f32 camera_position_z)
{
    i32 x, z;
    u32 i;
    i32 new_origin_x;
    i32 new_origin_z;

    i32 camera_tile_x = vast_tile_calculate_camera_tile(t, camera_position_x);
    i32 camera_tile_z = vast_tile_calculate_camera_tile(t, camera_position_z);

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
        vast_tiles_init(t, camera_position_x, camera_position_z, t->world_size);
        return;
    }

    /* Move East */
    while (t->origin_x < new_origin_x)
    {
        i32 target_x = t->origin_x + VAST_TILES_PER_SIDE;

        for (z = t->origin_z; z < t->origin_z + VAST_TILES_PER_SIDE; ++z)
        {
            u32 tile_index = vast_tile_index(target_x, z);

            t->tile_x[tile_index] = target_x;
            t->tile_z[tile_index] = z;
            t->dirty_indices[t->dirty_indices_count++] = (u16)tile_index;
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
            u32 tile_index = vast_tile_index(target_x, z);

            t->tile_x[tile_index] = target_x;
            t->tile_z[tile_index] = z;
            t->dirty_indices[t->dirty_indices_count++] = (u16)tile_index;
        }
    }

    /* Move South */
    while (t->origin_z < new_origin_z)
    {
        i32 target_z = t->origin_z + VAST_TILES_PER_SIDE;

        for (x = t->origin_x; x < t->origin_x + VAST_TILES_PER_SIDE; ++x)
        {
            u32 tile_index = vast_tile_index(x, target_z);

            t->tile_x[tile_index] = x;
            t->tile_z[tile_index] = target_z;
            t->dirty_indices[t->dirty_indices_count++] = (u16)tile_index;
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
            u32 tile_index = vast_tile_index(x, target_z);

            t->tile_x[tile_index] = x;
            t->tile_z[tile_index] = target_z;
            t->dirty_indices[t->dirty_indices_count++] = (u16)tile_index;
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