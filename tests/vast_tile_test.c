#define VAST_TILES_PER_SIDE 33
#include "../src/vast_tiles.h"

#define VAST_KB(s) ((s) / 1024)
#define VAST_MB(s) ((s) / 1024 / 1024)

#include "stdio.h"

i32 main(void)
{
    vast_tiles t = {0};

    i32 camera_x = 0;
    i32 camera_z = 0;

    i32 tile_world_size = 256; /* units in meters */

    i32 resolution_heightmap = 128;
    i32 resolution_normalmap = 1024;
    i32 bytes_per_pixel_heightmap = 4; /* GL_R32F */
    i32 bytes_per_pixel_normalmap = 3; /* GL_RGB8 */

    u32 i;

    u32 tile_lod_max = 0;
    u32 tile_lod_count[12] = {0};

    i32 total_bytes_heightmap = 0;
    i32 total_bytes_normalmap = 0;

    vast_tiles_init(&t, camera_x, camera_z);

    /* Get max lod */
    for (i = 0; i < VAST_TILES_TOTAL; ++i)
    {
        u32 lod = (u32)t.tile_lod[i];

        tile_lod_count[lod]++;

        if (lod > tile_lod_max)
        {
            tile_lod_max = lod;
        }
    }

    tile_lod_max += 1;

    printf("tile_tiles_per_side: %4d\n", VAST_TILES_PER_SIDE);
    printf("tile_tiles_total   : %4d\n", VAST_TILES_TOTAL);
    printf("tile_max_lod       : %4d\n", tile_lod_max);
    printf("tile_world_size    : %4d\n\n", tile_world_size);

    /* Print LOD stats */
    printf("%4s %15s %15s %11s %13s %13s\n", "LOD", "Height Texture", "Normal Texture", "Tile Count", "Height Bytes", "Normal Bytes");

    for (i = 0; i < tile_lod_max; ++i)
    {
        i32 lod_res_heightmap = resolution_heightmap / (1 << i);
        i32 lod_res_normalmap = resolution_normalmap / (1 << i);
        i32 lod_bytes_heightmap = ((lod_res_heightmap * lod_res_heightmap) * bytes_per_pixel_heightmap);
        i32 lod_bytes_normalmap = ((lod_res_normalmap * lod_res_normalmap) * bytes_per_pixel_normalmap);

        /* Multiply by the number of tiles */
        lod_bytes_heightmap *= (i32)tile_lod_count[i];
        lod_bytes_normalmap *= (i32)tile_lod_count[i];

        total_bytes_heightmap += lod_bytes_heightmap;
        total_bytes_normalmap += lod_bytes_normalmap;

        printf("%4d %15d %15d %11d %13d %13d\n", i, lod_res_heightmap, lod_res_normalmap, tile_lod_count[i], lod_bytes_heightmap, lod_bytes_normalmap);
    }

    /* Print memory Statistics */
    printf("\n");
    printf("heightmap_memory   : %10d mb, %10d kb, %10d bytes\n", VAST_MB(total_bytes_heightmap), VAST_KB(total_bytes_heightmap), total_bytes_heightmap);
    printf("normalmap_memory   : %10d mb, %10d kb, %10d bytes\n", VAST_MB(total_bytes_normalmap), VAST_KB(total_bytes_normalmap), total_bytes_normalmap);
    printf("total_memory       : %10d mb, %10d kb, %10d bytes\n\n", VAST_MB(total_bytes_heightmap + total_bytes_normalmap), VAST_KB(total_bytes_heightmap + total_bytes_normalmap), total_bytes_heightmap + total_bytes_normalmap);

    return 0;
}