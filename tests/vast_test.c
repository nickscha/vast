#include "../src/vast_tiles.h"
#include "../src/vast_math_basic.h"

#define VAST_KB(s) (((f64)s) / 1024.0)
#define VAST_MB(s) (((f64)s) / 1024.0 / 1024.0)

#include "stdio.h"

VAST_API VAST_INLINE void vast_test(u8 expression_result, s8 *expression_string, s8 *file, u32 line)
{
  printf("%s:%-4d (%s) = %s\n", file, line, expression_string, expression_result ? "OK" : "ERROR");
}

#define VAST_TEST(exp) vast_test((exp), (#exp), __FILE__, __LINE__);

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

VAST_API VAST_INLINE void vast_test_tile_memory_requirements(void)
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

  u32 tile_lod_count = 0;

  i32 total_bytes_heightmap = 0;
  i32 total_bytes_normalmap = 0;
  i32 total_vertices_needed = 0;

  vast_tiles_init(&t, camera_x, camera_z);

  tile_lod_count = (u32)t.tile_lod_max + 1;

  /* Print LOD stats */
  printf("\n%4s %15s %15s %11s %13s %13s\n", "LOD", "Height Texture", "Normal Texture", "Tile Count", "Height Bytes", "Normal Bytes");

  for (i = 0; i < tile_lod_count; ++i)
  {
    i32 lod_res_heightmap = resolution_heightmap / (1 << i);
    i32 lod_res_normalmap = resolution_normalmap / (1 << i);
    i32 lod_bytes_heightmap = lod_res_heightmap * lod_res_heightmap * bytes_per_pixel_heightmap;
    i32 lod_bytes_normalmap = lod_res_normalmap * lod_res_normalmap * bytes_per_pixel_normalmap;

    /* Multiply by the number of tiles */
    lod_bytes_heightmap *= (i32)t.tile_per_lod_count[i];
    lod_bytes_normalmap *= (i32)t.tile_per_lod_count[i];

    total_bytes_heightmap += lod_bytes_heightmap;
    total_bytes_normalmap += lod_bytes_normalmap;

    total_vertices_needed += lod_res_heightmap * lod_res_heightmap * (i32)t.tile_per_lod_count[i];

    printf("%4d %15d %15d %11d %13d %13d\n", i, lod_res_heightmap, lod_res_normalmap, t.tile_per_lod_count[i], lod_bytes_heightmap, lod_bytes_normalmap);
  }

  /* Print Statistics */
  printf("\n");
  printf("heightmap_memory   : %10.2f mb, %10.2f kb, %10d bytes\n", VAST_MB(total_bytes_heightmap), VAST_KB(total_bytes_heightmap), total_bytes_heightmap);
  printf("normalmap_memory   : %10.2f mb, %10.2f kb, %10d bytes\n", VAST_MB(total_bytes_normalmap), VAST_KB(total_bytes_normalmap), total_bytes_normalmap);
  printf("total_memory       : %10.2f mb, %10.2f kb, %10d bytes\n", VAST_MB(total_bytes_heightmap + total_bytes_normalmap), VAST_KB(total_bytes_heightmap + total_bytes_normalmap), total_bytes_heightmap + total_bytes_normalmap);
  printf("vast_tiles_struct  : %10.2f mb, %10.2f kb, %10d bytes\n\n", VAST_MB((i32)sizeof(vast_tiles)), VAST_KB((i32)sizeof(vast_tiles)), (i32)sizeof(vast_tiles));

  printf("tile_tiles_per_side: %10d\n", VAST_TILES_PER_SIDE);
  printf("tile_tiles_total   : %10d\n", VAST_TILES_TOTAL);
  printf("tile_lod_count     : %10d\n", tile_lod_count);
  printf("tile_world_size    : %10d\n", tile_world_size);
  printf("total_vertices     : %10d\n\n", total_vertices_needed);
}

VAST_API VAST_INLINE void vast_test_tile_transition_factor_calculation(void)
{
  vast_tiles t = {0};

  f32 tile_world_size = 256.0f;
  f32 camera_x = 0.0f;
  f32 camera_z = 0.0f;
  i32 camera_tile_x = (i32)vast_floorf((camera_x + tile_world_size * 0.5f) / tile_world_size);
  i32 camera_tile_z = (i32)vast_floorf((camera_z + tile_world_size * 0.5f) / tile_world_size);

  VAST_TEST(camera_tile_x == 0);
  VAST_TEST(camera_tile_z == 0);

  vast_tiles_init(&t, camera_tile_x, camera_tile_z);

  printf("\n");
  printf("tile_world_size: %10.4f |\n", (f64)tile_world_size);
  printf("camera_x       : %10.4f |\n", (f64)camera_x);
  printf("camera_z       : %10.4f |\n", (f64)camera_z);
  printf("camera_tile_x  : %10d |\n", camera_tile_x);
  printf("camera_tile_z  : %10d |\n", camera_tile_z);
  printf("tile_index     : %10d | tile_lod    : %10d\n", vast_tile_index(camera_tile_x, camera_tile_z), t.tile_lod_required[vast_tile_index(camera_tile_x, camera_tile_z)]);
  printf("tile_index_1_n : %10d | tile_lod_1_n: %10d\n", vast_tile_index(camera_tile_x, camera_tile_z - 1), t.tile_lod_required[vast_tile_index(camera_tile_x, camera_tile_z - 1)]);
  printf("tile_index_2_n : %10d | tile_lod_2_n: %10d\n", vast_tile_index(camera_tile_x, camera_tile_z - 2), t.tile_lod_required[vast_tile_index(camera_tile_x, camera_tile_z - 2)]);
  printf("\n");

  camera_z = -128.0f;
  VAST_TEST((i32)vast_floorf((camera_z + tile_world_size * 0.5f) / tile_world_size) == 0);
  camera_z = -129.0f;
  VAST_TEST((i32)vast_floorf((camera_z + tile_world_size * 0.5f) / tile_world_size) == -1);
}

VAST_API VAST_INLINE void vast_test_tile_camera_back_face_transition_factor(void)
{
  vast_tiles t = {0};

  f32 tile_world_size = 256.0f;
  f32 tile_world_size_half = tile_world_size * 0.5f;
  f32 camera_x = 0.0f;
  f32 camera_z = 0.0f;
  i32 camera_tile_x = (i32)vast_floorf((camera_x + tile_world_size * 0.5f) / tile_world_size);
  i32 camera_tile_z = (i32)vast_floorf((camera_z + tile_world_size * 0.5f) / tile_world_size);

  vast_tiles_init(&t, camera_tile_x, camera_tile_z);

  {
    f32 tile_x_min = (f32)camera_tile_x * tile_world_size - tile_world_size_half;
    f32 tile_x_max = (f32)tile_x_min + tile_world_size;
    f32 tile_z_min = (f32)camera_tile_z * tile_world_size - tile_world_size_half;
    f32 tile_z_max = (f32)tile_z_min + tile_world_size;

    printf("\n");
    printf("tile_world_size   : %10.4f |\n", (f64)tile_world_size);
    printf("camera_x          : %10.4f |\n", (f64)camera_x);
    printf("camera_z          : %10.4f |\n", (f64)camera_z);
    printf("camera_tile_x     : %10d |\n", camera_tile_x);
    printf("camera_tile_z     : %10d |\n", camera_tile_z);
    printf("tile_origin_x     : %10.4f |\n", (f64)t.origin_x);
    printf("tile_origin_z     : %10.4f |\n", (f64)t.origin_z);
    printf("tile_origin_x w   : %10.4f |\n", (f64)t.origin_x * (f64)tile_world_size);
    printf("tile_origin_z w   : %10.4f |\n", (f64)t.origin_z * (f64)tile_world_size);
    printf("tile_x_min        : %10.4f | tile_x_max     : %10.4f\n", (f64)tile_x_min, (f64)tile_x_max);
    printf("tile_z_min        : %10.4f | tile_z_max     : %10.4f\n", (f64)tile_z_min, (f64)tile_z_max);
  }

  /* Move south */
  camera_z = 129.0f;
  camera_tile_x = (i32)vast_floorf((camera_x + tile_world_size * 0.5f) / tile_world_size);
  camera_tile_z = (i32)vast_floorf((camera_z + tile_world_size * 0.5f) / tile_world_size);

  vast_tiles_update(&t, camera_tile_x, camera_tile_z);

  {
    f32 tile_x_min = (f32)camera_tile_x * tile_world_size - tile_world_size_half;
    f32 tile_x_max = (f32)tile_x_min + tile_world_size;
    f32 tile_z_min = (f32)camera_tile_z * tile_world_size - tile_world_size_half;
    f32 tile_z_max = (f32)tile_z_min + tile_world_size;

    printf("\n");
    printf("tile_world_size   : %10.4f |\n", (f64)tile_world_size);
    printf("camera_x          : %10.4f |\n", (f64)camera_x);
    printf("camera_z          : %10.4f |\n", (f64)camera_z);
    printf("camera_tile_x     : %10d |\n", camera_tile_x);
    printf("camera_tile_z     : %10d |\n", camera_tile_z);
    printf("tile_origin_x     : %10.4f |\n", (f64)t.origin_x);
    printf("tile_origin_z     : %10.4f |\n", (f64)t.origin_z);
    printf("tile_origin_x w   : %10.4f |\n", (f64)t.origin_x * (f64)tile_world_size);
    printf("tile_origin_z w   : %10.4f |\n", (f64)t.origin_z * (f64)tile_world_size);
    printf("tile_x_min        : %10.4f | tile_x_max     : %10.4f\n", (f64)tile_x_min, (f64)tile_x_max);
    printf("tile_z_min        : %10.4f | tile_z_max     : %10.4f\n", (f64)tile_z_min, (f64)tile_z_max);
  }
}

i32 main(void)
{
  vast_test_tile_memory_requirements();
  vast_test_tile_transition_factor_calculation();
  vast_test_tile_camera_back_face_transition_factor();

  return 0;
}