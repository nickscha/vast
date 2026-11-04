#define VAST_TILES_PER_SIDE 33
#include "../src/vast_tiles.h"

#define VAST_KB(s) (((f64)s) / 1024.0)
#define VAST_MB(s) (((f64)s) / 1024.0 / 1024.0)

#include "stdio.h"

/* #############################################################################
 * # [TEST] Calculate Memory / Vertex / Tile requirements
 * #############################################################################

   LOD  Height Texture  Normal Texture  Tile Count  Height Bytes  Normal Bytes
     0             128            1024           9        589824      28311552
     1              64             512          40        655360      31457280
     2              32             256          72        294912      14155776
     3              16             128         104        106496       5111808
     4               8              64         136         34816       1671168
     5               4              32         728         46592       2236416

  heightmap_memory   :       1.65 mb,    1687.50 kb,    1728000 bytes
  normalmap_memory   :      79.10 mb,   81000.00 kb,   82944000 bytes
  total_memory       :      80.75 mb,   82687.50 kb,   84672000 bytes

  tile_tiles_per_side:         33
  tile_tiles_total   :       1089
  tile_max_lod       :          6
  tile_world_size    :        256
  total_vertices     :     432000

 */
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
    i32 total_vertices_needed = 0;

    vast_tiles_init(&t, camera_x, camera_z);

    tile_lod_max = (u32)t.tile_lod_max + 1;

    /* Get max lod */
    for (i = 0; i < VAST_TILES_TOTAL; ++i)
    {
        u32 lod = (u32)t.tile_lod[i];
        tile_lod_count[lod]++;
    }

    /* Print LOD stats */
    printf("\n%4s %15s %15s %11s %13s %13s\n", "LOD", "Height Texture", "Normal Texture", "Tile Count", "Height Bytes", "Normal Bytes");

    for (i = 0; i < tile_lod_max; ++i)
    {
        i32 lod_res_heightmap = resolution_heightmap / (1 << i);
        i32 lod_res_normalmap = resolution_normalmap / (1 << i);
        i32 lod_bytes_heightmap = lod_res_heightmap * lod_res_heightmap * bytes_per_pixel_heightmap;
        i32 lod_bytes_normalmap = lod_res_normalmap * lod_res_normalmap * bytes_per_pixel_normalmap;

        /* Multiply by the number of tiles */
        lod_bytes_heightmap *= (i32)tile_lod_count[i];
        lod_bytes_normalmap *= (i32)tile_lod_count[i];

        total_bytes_heightmap += lod_bytes_heightmap;
        total_bytes_normalmap += lod_bytes_normalmap;

        total_vertices_needed += lod_res_heightmap * lod_res_heightmap * (i32)tile_lod_count[i];

        printf("%4d %15d %15d %11d %13d %13d\n", i, lod_res_heightmap, lod_res_normalmap, tile_lod_count[i], lod_bytes_heightmap, lod_bytes_normalmap);
    }

    /* Print Statistics */
    printf("\n");
    printf("heightmap_memory   : %10.2f mb, %10.2f kb, %10d bytes\n", VAST_MB(total_bytes_heightmap), VAST_KB(total_bytes_heightmap), total_bytes_heightmap);
    printf("normalmap_memory   : %10.2f mb, %10.2f kb, %10d bytes\n", VAST_MB(total_bytes_normalmap), VAST_KB(total_bytes_normalmap), total_bytes_normalmap);
    printf("total_memory       : %10.2f mb, %10.2f kb, %10d bytes\n\n", VAST_MB(total_bytes_heightmap + total_bytes_normalmap), VAST_KB(total_bytes_heightmap + total_bytes_normalmap), total_bytes_heightmap + total_bytes_normalmap);

    printf("tile_tiles_per_side: %10d\n", VAST_TILES_PER_SIDE);
    printf("tile_tiles_total   : %10d\n", VAST_TILES_TOTAL);
    printf("tile_max_lod       : %10d\n", tile_lod_max);
    printf("tile_world_size    : %10d\n", tile_world_size);
    printf("total_vertices     : %10d\n\n", total_vertices_needed);

    return 0;
}