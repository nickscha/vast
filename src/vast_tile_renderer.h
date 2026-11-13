#ifndef VAST_TILE_RENDERER_H
#define VAST_TILE_RENDERER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)    */
#include "vast.h"                     /* Platform State                */
#include "vast_string_builder.h"      /* String printing               */
#include "vast_profiler.h"            /* Profiler metrics              */
#include "vast_tiles.h"               /* Toriodal Tiling Grid logic    */
#include "vast_camera.h"              /* Camera                        */
#include "vast_geometry.h"            /* Calc. Grid with Skirts        */
#include "vast_math_linear_algebra.h" /* Linear Algebra                */
#include "vast_opengl_api.h"          /* OpenGL Flags                  */

/* #############################################################################
 * # [SECTION] Structs and Data
 * #############################################################################
 */
typedef struct vast_tile_shader_heightmap
{
    shader_header header;

    i32 loc_tile_offset;
    i32 loc_tile_size;
    i32 loc_texture_size;

} vast_tile_shader_heightmap;

typedef struct vast_tile_shader_normalmap
{
    shader_header header;

    i32 loc_tile_offset;
    i32 loc_tile_size;
    i32 loc_texture_size;

} vast_tile_shader_normalmap;

typedef struct vast_tile_shader_final
{
    shader_header header;

    i32 loc_tile_offset;
    i32 loc_view_projection;
    i32 loc_is_dirty;
    i32 loc_heightmap;
    i32 loc_normalmap;
    i32 loc_texture_array_layer;
    i32 loc_iResolution;
    i32 loc_iTime;
    i32 loc_iMouse;
    i32 loc_camera;
    i32 loc_sun_dir;
    i32 loc_camera_view_dir;
    i32 loc_tile_size;
    i32 loc_grid_res;
    i32 loc_visualization_mode;

} vast_tile_shader_final;

typedef struct vast_tile_instance_data
{
    f32 offset_x;
    f32 offset_z;
    f32 texture_array_layer;
    f32 is_dirty;
    f32 transition_factor;

} vast_tile_instance_data;

static s8 *vast_tile_texture_vertex_data =
    "#version 330 core\n"
    "out vec2 v_uv;\n"
    "const vec2 verts[3] = vec2[](vec2(-1.0, -1.0),vec2( 3.0, -1.0),vec2(-1.0,  3.0));\n"
    "void main()\n"
    "{\n"
    "  vec2 p = verts[gl_VertexID];\n"
    "  v_uv = p * 0.5 + 0.5;\n"
    "  gl_Position = vec4(p, 0.0, 1.0);\n"
    "}";

/* #############################################################################
 * # [SECTION] Tile Renderer
 * #############################################################################
 */
#define VAST_TILE_GRID_SIZE 65
#define VAST_TILE_MAX_INDICES ((VAST_TILE_GRID_SIZE - 1) * (VAST_TILE_GRID_SIZE - 1) * 6)

VAST_API void vast_tile_shader_heightmap_load(vast_platform_state *state, vast_tile_shader_heightmap *tiles_hm_shader)
{
    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/vast_tile_hm.fs");
    vast_opengl_api *gl = &state->graphics.opengl;

    if (!shader_data_fragment)
    {
        state->memory.offset = 0;
        return;
    }

    if (opengl_shader_load(&tiles_hm_shader->header, vast_tile_texture_vertex_data, (s8 *)shader_data_fragment, state->api.io_print, gl))
    {
        tiles_hm_shader->loc_tile_offset = gl->glGetUniformLocation(tiles_hm_shader->header.program, "u_tile_offset");
        tiles_hm_shader->loc_tile_size = gl->glGetUniformLocation(tiles_hm_shader->header.program, "u_tile_size");
        tiles_hm_shader->loc_texture_size = gl->glGetUniformLocation(tiles_hm_shader->header.program, "u_texture_size");
    }
    else
    {
        state->api.io_print("Cannot compile tile heightmap shaders!\n");
    }

    state->memory.offset = 0;
}

VAST_API void vast_tile_shader_normalmap_load(vast_platform_state *state, vast_tile_shader_normalmap *tiles_nm_shader)
{
    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/vast_tile_nm.fs");
    vast_opengl_api *gl = &state->graphics.opengl;

    if (!shader_data_fragment)
    {
        state->memory.offset = 0;
        return;
    }

    if (opengl_shader_load(&tiles_nm_shader->header, vast_tile_texture_vertex_data, (s8 *)shader_data_fragment, state->api.io_print, gl))
    {
        tiles_nm_shader->loc_tile_offset = gl->glGetUniformLocation(tiles_nm_shader->header.program, "u_tile_offset");
        tiles_nm_shader->loc_tile_size = gl->glGetUniformLocation(tiles_nm_shader->header.program, "u_tile_size");
        tiles_nm_shader->loc_texture_size = gl->glGetUniformLocation(tiles_nm_shader->header.program, "u_texture_size");
    }
    else
    {
        state->api.io_print("Cannot compile tile normal map shaders!\n");
    }

    state->memory.offset = 0;
}

VAST_API void vast_tile_shader_final_load(vast_platform_state *state, vast_tile_shader_final *tiles_shader)
{

    u8 *shader_data_vertex = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/vast_tiles.vs");
    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/vast_tiles.fs");
    vast_opengl_api *gl = &state->graphics.opengl;

    if (!shader_data_vertex || !shader_data_fragment)
    {
        state->memory.offset = 0;
        return;
    }

    if (opengl_shader_load(&tiles_shader->header, (s8 *)shader_data_vertex, (s8 *)shader_data_fragment, state->api.io_print, gl))
    {
        tiles_shader->loc_tile_offset = gl->glGetUniformLocation(tiles_shader->header.program, "u_tile_offset");
        tiles_shader->loc_view_projection = gl->glGetUniformLocation(tiles_shader->header.program, "u_vp");
        tiles_shader->loc_is_dirty = gl->glGetUniformLocation(tiles_shader->header.program, "u_is_dirty");
        tiles_shader->loc_heightmap = gl->glGetUniformLocation(tiles_shader->header.program, "u_heightmap");
        tiles_shader->loc_normalmap = gl->glGetUniformLocation(tiles_shader->header.program, "u_normalmap");
        tiles_shader->loc_texture_array_layer = gl->glGetUniformLocation(tiles_shader->header.program, "u_texture_array_layer");
        tiles_shader->loc_tile_size = gl->glGetUniformLocation(tiles_shader->header.program, "u_tile_size");
        tiles_shader->loc_grid_res = gl->glGetUniformLocation(tiles_shader->header.program, "u_grid_res");
        tiles_shader->loc_visualization_mode = gl->glGetUniformLocation(tiles_shader->header.program, "u_visualization_mode");

        tiles_shader->loc_iResolution = gl->glGetUniformLocation(tiles_shader->header.program, "iResolution");
        tiles_shader->loc_iTime = gl->glGetUniformLocation(tiles_shader->header.program, "iTime");
        tiles_shader->loc_iMouse = gl->glGetUniformLocation(tiles_shader->header.program, "iMouse");
        tiles_shader->loc_sun_dir = gl->glGetUniformLocation(tiles_shader->header.program, "sunDir");
        tiles_shader->loc_camera = gl->glGetUniformLocation(tiles_shader->header.program, "iCamera");
        tiles_shader->loc_camera_view_dir = gl->glGetUniformLocation(tiles_shader->header.program, "iViewDir");
    }
    else
    {
        state->api.io_print("Cannot compile tiles shaders!\n");
    }

    state->memory.offset = 0;
}

#ifndef VAST_RELEASE
VAST_API void vast_tile_shader_hot_reload(
    vast_platform_state *state,
    vast_tiles *tiles,
    vast_tile_shader_final *tiles_shader,
    vast_tile_shader_heightmap *tiles_hm_shader,
    vast_tile_shader_normalmap *tiles_nm_shader,
    i32 camera_tile_x,
    i32 camera_tile_z)
{
    if (state->timing.frame_count % 30 == 0)
    {
        static u8 hot_reload_initialized = 0;

        static s8 *tile_shader_heightmap = "shaders/vast_tile_hm.fs";
        static s8 *tile_shader_normalmap = "shaders/vast_tile_nm.fs";
        static s8 *tile_shader_final = "shaders/vast_tiles.fs";
        static u32 tile_shader_heightmap_last = 0;
        static u32 tile_shader_normalmap_last = 0;
        static u32 tile_shader_final_last = 0;

        u32 tile_shader_heightmap_now = 0;
        u32 tile_shader_normalmap_now = 0;
        u32 tile_shader_final_now = 0;

        VAST_PROFILER_BEGIN(tile_hot_reload);

        /* Get initial modifcation dates of shaders for hot reloading */
        if (!hot_reload_initialized)
        {
            state->api.io_file_mod_signature(tile_shader_heightmap, &tile_shader_heightmap_last);
            state->api.io_file_mod_signature(tile_shader_normalmap, &tile_shader_normalmap_last);
            state->api.io_file_mod_signature(tile_shader_final, &tile_shader_final_last);

            hot_reload_initialized = 1;
        }

        /* Get current modification timestamp of files */
        state->api.io_file_mod_signature(tile_shader_heightmap, &tile_shader_heightmap_now);
        state->api.io_file_mod_signature(tile_shader_normalmap, &tile_shader_normalmap_now);
        state->api.io_file_mod_signature(tile_shader_final, &tile_shader_final_now);

        if (tile_shader_heightmap_now != tile_shader_heightmap_last)
        {
            vast_tile_shader_heightmap_load(state, tiles_hm_shader);
            vast_tiles_init(tiles, camera_tile_x, camera_tile_z); /* Regenerate terrain fully */
            tile_shader_heightmap_last = tile_shader_heightmap_now;
        }

        if (tile_shader_normalmap_now != tile_shader_normalmap_last)
        {
            vast_tile_shader_normalmap_load(state, tiles_nm_shader);
            vast_tiles_init(tiles, camera_tile_x, camera_tile_z); /* Regenerate terrain fully */
            tile_shader_normalmap_last = tile_shader_normalmap_now;
        }

        if (tile_shader_final_now != tile_shader_final_last)
        {
            vast_tile_shader_final_load(state, tiles_shader);
            tile_shader_final_last = tile_shader_final_now;
        }

        VAST_PROFILER_END(tile_hot_reload);
    }
}
#endif

VAST_API u8 vast_tile_is_visible(
    vast_frustum *f,
    f32 tileX,
    f32 tileZ,
    f32 tileSize,
    f32 minHeight,
    f32 maxHeight)
{
    f32 minX = tileX * tileSize - tileSize * 0.5f;
    f32 maxX = tileX * tileSize + tileSize * 0.5f;

    f32 minZ = tileZ * tileSize - tileSize * 0.5f;
    f32 maxZ = tileZ * tileSize + tileSize * 0.5f;

    i32 i;

    for (i = 0; i < VAST_FRUSTUM_COUNT; ++i)
    {
        vast_vec4 *p = &f->planes[i];

        f32 px = (p->x >= 0.0f) ? maxX : minX;
        f32 py = (p->y >= 0.0f) ? maxHeight : minHeight;
        f32 pz = (p->z >= 0.0f) ? maxZ : minZ;

        if (p->x * px + p->y * py + p->z * pz + p->w < 0.0f)
        {
            return 0;
        }
    }

    return 1;
}

/* #############################################################################
 * # [SECTION] Texture Pool Slot Allocations and Ownership Tracking
 * #############################################################################
 */
typedef struct vast_tile_pool_slot
{
    i32 current_lod;
    i32 physical_layer;
} vast_tile_pool_slot;

/* TODO(nickscha) use calculated max lod level from tiles struct */
#define LOD_COUNT 6

/* TODO(nickscha) hardcoded for now */
static i32 lod_texture_pool_size[LOD_COUNT] = {
    /* steady */ 9 + /* burst headroom */ 7,    /* LOD 0: 128x128 height, 1024x1024 normal */
    /* steady */ 40 + /* burst headroom */ 24,  /* LOD 1:  64x64  height,  512x512  normal */
    /* steady */ 72 + /* burst headroom */ 24,  /* LOD 2:  32x32  height,  256x256  normal */
    /* steady */ 104 + /* burst headroom */ 24, /* LOD 3:  16x16  height,  128x128  normal */
    /* steady */ 136 + /* burst headroom */ 56, /* LOD 4:   8x8   height,   64x64   normal */
    /* steady */ 728 + /* burst headroom */ 22  /* LOD 5:   4x4   height,   32x32   normal */
};

static i32 texture_layer[VAST_TILES_TOTAL] = {0};

/* TODO(nickscha): Not bulletproof yet */
VAST_API i32 vast_tile_aquire_texture_array_layer(vast_tiles *t, u32 tile_index)
{
    static u8 initialized = 0;

    static u8 texture_layer_used[LOD_COUNT][VAST_TILES_TOTAL] = {0};
    static i32 texture_layer_next_free[LOD_COUNT] = {0};

    u32 i;

    i32 lod_current;
    i32 lod_required;
    i32 texture_layer_current;
    i32 texture_array_layer_capacity_required;
    i32 texture_layer_assigned;

    if (!initialized)
    {

        for (i = 0; i < VAST_TILES_TOTAL; ++i)
        {
            texture_layer[i] = -1;
        }

        initialized = 1;
    }

    lod_current = t->tile_lod_current[tile_index];
    lod_required = t->tile_lod_required[tile_index];

    /* Nothing changes */
    if (lod_current == lod_required && texture_layer[tile_index] >= 0)
    {
        return texture_layer[tile_index];
    }

    if (lod_current == -1)
    {
        lod_current = lod_required;
    }

    texture_layer_current = texture_layer[tile_index];
    texture_array_layer_capacity_required = lod_texture_pool_size[lod_required];

    /* Current LOD has texture layer assigned. Free it. */
    if (texture_layer_current >= 0)
    {
        texture_layer_used[lod_current][texture_layer_current] = 0;
        texture_layer_next_free[lod_current] = texture_layer_current;
    }

    texture_layer_assigned = texture_layer_next_free[lod_required];

    /* Mark as used */
    texture_layer_used[lod_required][texture_layer_assigned] = 1;

    /* Find the next free texture layer */
    for (i = 0; i < (u32)texture_array_layer_capacity_required; ++i)
    {
        if (!texture_layer_used[lod_required][i])
        {
            texture_layer_next_free[lod_required] = (i32)i;
            break;
        }
    }

    texture_layer[tile_index] = texture_layer_assigned;

    return texture_layer_assigned;
}

/* #############################################################################
 * # [SECTION] Tile Rendering
 * #############################################################################
 */
VAST_API VAST_INLINE f32 vast_tile_camera_backface_edge_factor(
    f32 camera_x,
    f32 camera_z,
    f32 forward_x,
    f32 forward_z,
    f32 min_x,
    f32 min_z,
    f32 max_x,
    f32 max_z,
    f32 threshold)
{
    f32 distance;
    f32 threshold_distance;
    f32 dx;
    f32 dz;

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
        dx = camera_x - min_x;

        if (dx < distance)
        {
            distance = dx;
        }
    }
    else if (forward_x < 0.0f)
    {
        dx = max_x - camera_x;

        if (dx < distance)
        {
            distance = dx;
        }
    }

    /* Opposite Z direction. */
    if (forward_z > 0.0f)
    {
        dz = camera_z - min_z;

        if (dz < distance)
        {
            distance = dz;
        }
    }
    else if (forward_z < 0.0f)
    {
        dz = max_z - camera_z;

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

    if (distance > 1.0f)
    {
        distance = 1.0f;
    }

    if (distance < 0.0f)
    {
        distance = 0.0f;
    }

    return distance;
}

/* TODO(nickscha): texture pool capacity = tile_count + burst_headroom */
VAST_API void vast_tile_render(vast_platform_state *state, vast_camera *camera, vast_vec3 sun_dir, u32 *draw_calls)
{
    static u8 tiles_initialized = 0;
    static vast_tiles t = {0};
    static i32 camera_tile_x = 0;
    static i32 camera_tile_z = 0;

    static vast_tile_shader_final tiles_shader = {0};
    static vast_tile_shader_heightmap tiles_hm_shader = {0};
    static vast_tile_shader_normalmap tiles_nm_shader = {0};

    static f32 tile_world_size = 256.0f;
    static f32 tile_world_size_half = 256.0f * 0.5f;

    static u32 grid_vaos[LOD_COUNT];
    static u32 grid_vbos[LOD_COUNT];
    static u32 grid_ibos[LOD_COUNT];
    static i32 gridIndexCounts[LOD_COUNT] = {0};
    static u32 lod_instance_vbos[LOD_COUNT];
    static f32 tile_transition_timer[VAST_TILES_TOTAL] = {0};

    static u32 scratch_vao;
    static u32 scratch_fbo;

    /* Per-LOD Texture Array Handles */
    static u32 lod_heightmap_arrays[LOD_COUNT];
    static u32 lod_normalmap_arrays[LOD_COUNT];

    vast_opengl_api *gl = &state->graphics.opengl;

    if (!tiles_initialized)
    {
        /* Setup tiles */
        camera_tile_x = (i32)vast_floorf((camera->position.x + tile_world_size * 0.5f) / tile_world_size);
        camera_tile_z = (i32)vast_floorf((camera->position.z + tile_world_size * 0.5f) / tile_world_size);

        VAST_PROFILER_BEGIN(tile_init);
        vast_tiles_init(&t, camera_tile_x, camera_tile_z);
        VAST_PROFILER_END(tile_init);

        {
            u32 i;

            for (i = 0; i < VAST_TILES_TOTAL; ++i)
            {
                tile_transition_timer[i] = 1.0f;
            }
        }

        /* Setup tiles texture pools */
        {
            u32 lod;

            for (lod = 0; lod < LOD_COUNT; ++lod)
            {
                i32 resolution_heightmap = ((VAST_TILE_GRID_SIZE - 1) / (1 << lod)) + 1;
                i32 resolution_normalmap = 1024 / (1 << lod);
                i32 texture_pool_size = lod_texture_pool_size[lod];

                /* Heightmap Texture Array (GL_R32F / 4 byte) */
                gl->glGenTextures(1, &lod_heightmap_arrays[lod]);
                gl->glBindTexture(GL_TEXTURE_2D_ARRAY, lod_heightmap_arrays[lod]);
                gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, resolution_heightmap, resolution_heightmap, texture_pool_size, 0, GL_RED, GL_FLOAT, VAST_NULL);
                gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                /* Normalmap Texture Array (GL_RGB8 / 3 byte) */
                gl->glGenTextures(1, &lod_normalmap_arrays[lod]);
                gl->glBindTexture(GL_TEXTURE_2D_ARRAY, lod_normalmap_arrays[lod]);
                gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, resolution_normalmap, resolution_normalmap, texture_pool_size, 0, GL_RGB, GL_UNSIGNED_BYTE, VAST_NULL);
                gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }

        /* Setup tiles shaders */
        vast_tile_shader_heightmap_load(state, &tiles_hm_shader);
        vast_tile_shader_normalmap_load(state, &tiles_nm_shader);
        vast_tile_shader_final_load(state, &tiles_shader);

        /* Dummy vertex and framebuffer object */
        gl->glGenVertexArrays(1, &scratch_vao);
        gl->glGenFramebuffers(1, &scratch_fbo);

        /* Generate Grid with Skirts for all LOD */
        {
            u32 i;

            for (i = 0; i < (u32)t.tile_lod_max + 1; ++i)
            {
                i32 grid_res = ((VAST_TILE_GRID_SIZE - 1) / (1 << (u32)i)) + 1;
                vast_geometry_vertex_skirt vertices[VAST_TILE_GRID_SIZE * VAST_TILE_GRID_SIZE + 600];
                u16 indices[VAST_TILE_GRID_SIZE * VAST_TILE_GRID_SIZE * 6 + 1200];

                gridIndexCounts[i] = vast_geometry_grid_skirt(grid_res, vertices, indices);

                gl->glGenVertexArrays(1, &grid_vaos[i]);
                gl->glBindVertexArray(grid_vaos[i]);

                /* 1. Vertex Buffer */
                gl->glGenBuffers(1, &grid_vbos[i]);
                gl->glBindBuffer(GL_ARRAY_BUFFER, grid_vbos[i]);
                gl->glBufferData(GL_ARRAY_BUFFER, (i32)sizeof(vast_geometry_vertex_skirt) * (grid_res * grid_res + 600), vertices, GL_STATIC_DRAW);

                /* 2. Index Buffer */
                gl->glGenBuffers(1, &grid_ibos[i]);
                gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ibos[i]);
                gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndexCounts[i] * (i32)sizeof(u16), indices, GL_STATIC_DRAW);

                /* Attribute 0: UV (vec2) - Per-vertex */
                gl->glEnableVertexAttribArray(0);
                gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vast_geometry_vertex_skirt), VAST_NULL);

                /* Attribute 1: Is Skirt Flag (float) - Per-vertex */
                gl->glEnableVertexAttribArray(1);
                gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(vast_geometry_vertex_skirt), (void *)(sizeof(f32) * 2));

                /* 3. Instance VBO */
                gl->glGenBuffers(1, &lod_instance_vbos[i]);
                gl->glBindBuffer(GL_ARRAY_BUFFER, lod_instance_vbos[i]);
                gl->glBufferData(GL_ARRAY_BUFFER, (i32)sizeof(vast_tile_instance_data) * VAST_TILES_TOTAL, VAST_NULL, GL_DYNAMIC_DRAW);

                /* Attribute 2: Instance Offset (vec2) - Per-instance */
                gl->glEnableVertexAttribArray(2);
                gl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vast_tile_instance_data), VAST_NULL);
                gl->glVertexAttribDivisor(2, 1);

                /* Attribute 3: Instance Data (vec2: texture_layer, is_dirty, transition_factor) - Per-instance */
                gl->glEnableVertexAttribArray(3);
                gl->glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vast_tile_instance_data), (void *)(sizeof(f32) * 2));
                gl->glVertexAttribDivisor(3, 1);

                gl->glBindVertexArray(0);
            }
        }

        tiles_initialized = 1;
    }

#ifndef VAST_RELEASE
    vast_tile_shader_hot_reload(state, &t, &tiles_shader, &tiles_hm_shader, &tiles_nm_shader, camera_tile_x, camera_tile_z);
#endif

    /* Test camera movement tile change */
    if (state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_I] && !state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_I])
    {
        static u8 camera_sim_enabled = 0;

        if (!camera_sim_enabled)
        {
            camera->position = vast_vec3_init(-128.0f, 146.0f, 51.0f);
            camera->forward = vast_vec3_init(1.0f, 0.0f, 0.0f);
            camera->right = vast_vec3_init(0.0f, 0.0f, 1.0f);
            camera->yaw = 2.8f;
            camera->fov = 29.0f;
            camera_sim_enabled = 1;
        }
        else
        {
            camera->position.x = camera->position.x < -128.0f ? -128.0f : -129.0f;
        }
    }

    /* Update camera tile based on camera world position */
    camera_tile_x = (i32)vast_floorf((camera->position.x + tile_world_size * 0.5f) / tile_world_size);
    camera_tile_z = (i32)vast_floorf((camera->position.z + tile_world_size * 0.5f) / tile_world_size);

    /* Check for new dirty tiles */
    VAST_PROFILER_BEGIN(tile_update);
    vast_tiles_update(&t, camera_tile_x, camera_tile_z);
    VAST_PROFILER_END(tile_update);

    /* Process dirty tiles */
    VAST_PROFILER_BEGIN(tile_process_dirty);
    {
        /* When no tiles calculated process everything in one frame otherwise introduce frame pacing */
        i32 updates_per_frame = t.dirty_indices_count == VAST_TILES_TOTAL ? VAST_TILES_TOTAL : 6;

        while (updates_per_frame > 0 && t.dirty_indices_count > 0)
        {
            u32 last_idx = t.dirty_indices_count - 1;
            u32 tile_idx = t.dirty_indices[last_idx];

            i8 tile_lod_required = t.tile_lod_required[tile_idx];

            i32 height_tex_size = ((VAST_TILE_GRID_SIZE - 1) / (1 << (u32)tile_lod_required)) + 1;
            u32 normal_tex_size = 1024 / (1 << (u32)tile_lod_required);

            i32 texture_array_layer = vast_tile_aquire_texture_array_layer(&t, tile_idx);

            gl->glBindVertexArray(scratch_vao);
            gl->glBindFramebuffer(GL_FRAMEBUFFER, scratch_fbo);

            /* Render Height Map to Framebuffer */
            VAST_PROFILER_BEGIN(tile_render_heightmap);
            {
                gl->glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, lod_heightmap_arrays[(u32)tile_lod_required], 0, texture_array_layer);
                gl->glViewport(0, 0, (i32)height_tex_size, (i32)height_tex_size);

                gl->glUseProgram(tiles_hm_shader.header.program);
                gl->glUniform2f(tiles_hm_shader.loc_tile_offset, (f32)t.tile_x[tile_idx], (f32)t.tile_z[tile_idx]);
                gl->glUniform1f(tiles_hm_shader.loc_texture_size, (f32)height_tex_size);

                /* TODO(nickscha): only need to be set once and not every frame! */
                gl->glUniform1f(tiles_hm_shader.loc_tile_size, tile_world_size);

                gl->glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            VAST_PROFILER_END(tile_render_heightmap);

            /* Render Normal Map to Framebuffer */
            VAST_PROFILER_BEGIN(tile_render_normalmap);
            {
                gl->glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, lod_normalmap_arrays[(u32)tile_lod_required], 0, texture_array_layer);
                gl->glViewport(0, 0, (i32)normal_tex_size, (i32)normal_tex_size);

                gl->glUseProgram(tiles_nm_shader.header.program);
                gl->glUniform2f(tiles_nm_shader.loc_tile_offset, (f32)t.tile_x[tile_idx], (f32)t.tile_z[tile_idx]);
                gl->glUniform1f(tiles_nm_shader.loc_texture_size, (f32)normal_tex_size);

                /* TODO(nickscha): only need to be set once and not every frame! */
                gl->glUniform1f(tiles_nm_shader.loc_tile_size, tile_world_size);

                gl->glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            VAST_PROFILER_END(tile_render_normalmap);

            /* Restore Viewport state */
            gl->glViewport(0, 0, (i32)state->window.width, (i32)state->window.height);
            gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

            t.tile_lod_current[tile_idx] = tile_lod_required;
            t.dirty_indices_count--;
            updates_per_frame--;
        }
    }
    VAST_PROFILER_END(tile_process_dirty);

    /* Render Tiles */
    VAST_PROFILER_BEGIN(tile_render);
    {
        static u8 wireframe_enabled = 0;
        static u32 visualization_mode = 0;
        static vast_tile_instance_data instance_buffer[VAST_TILES_TOTAL];

        u32 lod;

        vast_mat4x4 projection = vast_mat4x4_perspective(VAST_DEG_TO_RAD(camera->fov), (f32)state->window.width / (f32)state->window.height, 0.1f, 8192.0f);
        vast_mat4x4 view = vast_mat4x4_look_at(camera->position, vast_vec3_add(camera->position, camera->forward), camera->up);
        vast_mat4x4 mvp = vast_mat4x4_mul(projection, view);
        vast_frustum frustum = vast_frustum_extract(mvp);

        *draw_calls = 0;

        if (state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_TAB] && !state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_TAB])
        {
            wireframe_enabled = !wireframe_enabled;
        }

        if (state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_V] && !state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_V])
        {
            visualization_mode++;

            if (visualization_mode > 4)
            {
                visualization_mode = 0;
            }
        }

        gl->glEnable(GL_CULL_FACE);
        gl->glCullFace(GL_BACK);

        gl->glUseProgram(tiles_shader.header.program);
        gl->glUniformMatrix4fv(tiles_shader.loc_view_projection, 1, GL_FALSE, mvp.e);
        gl->glUniform3f(tiles_shader.loc_iResolution, (f32)state->window.width, (f32)state->window.height, 1.0f);
        gl->glUniform1f(tiles_shader.loc_iTime, (f32)state->timing.time_elapsed);
        gl->glUniform3f(tiles_shader.loc_iMouse, (f32)state->input.mouse.x, (f32)state->input.mouse.y, 0.0f);
        gl->glUniform3f(tiles_shader.loc_camera, camera->position.x, camera->position.y, camera->position.z);
        gl->glUniform3f(tiles_shader.loc_camera_view_dir, camera->forward.x, camera->forward.y, camera->forward.z);
        gl->glUniform3f(tiles_shader.loc_sun_dir, sun_dir.x, sun_dir.y, sun_dir.z);
        gl->glUniform1f(tiles_shader.loc_visualization_mode, (f32)visualization_mode);

        gl->glUniform1f(tiles_shader.loc_tile_size, tile_world_size);
        gl->glUniform1i(tiles_shader.loc_heightmap, 0);
        gl->glUniform1i(tiles_shader.loc_normalmap, 1);

        gl->glPolygonMode(GL_FRONT_AND_BACK, wireframe_enabled ? GL_LINE : GL_FILL);

        for (lod = 0; lod < LOD_COUNT; ++lod)
        {

            i32 instance_count = 0;
            u32 i;
            i32 grid_res = ((VAST_TILE_GRID_SIZE - 1) / (1 << lod)) + 1;

            for (i = 0; i < VAST_TILES_TOTAL; ++i)
            {
                i32 tile_lod_current = t.tile_lod_current[i];
                i32 texture_array_layer = texture_layer[i];

                if (tile_lod_current != (i32)lod || texture_array_layer < 0)
                {
                    continue;
                }

                if (!vast_tile_is_visible(&frustum, (f32)t.tile_x[i], (f32)t.tile_z[i], tile_world_size, -200.0f, 600.0f))
                {
                    /* Stop LOD geomorphing since the tile is not visible */
                    tile_transition_timer[i] = 1.0f;
                    continue;
                }

                /* LOD Geomorphing: Fine to Coarse transition when moving backwards */
                if (vast_tile_is_on_lod_outer_edge(&t, i))
                {
                    f32 tile_x_min = (f32)camera_tile_x * tile_world_size - tile_world_size_half;
                    f32 tile_x_max = (f32)tile_x_min + tile_world_size;
                    f32 tile_z_min = (f32)camera_tile_z * tile_world_size - tile_world_size_half;
                    f32 tile_z_max = (f32)tile_z_min + tile_world_size;

                    f32 threshold = 0.1f;

                    /* 1.0f at boundary - 0.0f at edge */
                    f32 camera_tile_factor = vast_tile_camera_backface_edge_factor(
                        camera->position.x, camera->position.z,
                        camera->forward.x, camera->forward.z,
                        tile_x_min, tile_z_min, tile_x_max, tile_z_max,
                        threshold);

                    (void)camera_tile_factor;

                    /*tile_transition_timer[i] = camera_tile_factor;*/
                }

                instance_buffer[instance_count].offset_x = (f32)t.tile_x[i];
                instance_buffer[instance_count].offset_z = (f32)t.tile_z[i];
                instance_buffer[instance_count].texture_array_layer = (f32)texture_array_layer;
                instance_buffer[instance_count].is_dirty = (f32)vast_tile_is_dirty(&t, i);
                instance_buffer[instance_count].transition_factor = tile_transition_timer[i];
                instance_count++;
            }

            if (instance_count == 0)
            {
                continue;
            }

            gl->glUniform1f(tiles_shader.loc_grid_res, (f32)grid_res);

            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D_ARRAY, lod_heightmap_arrays[lod]);

            gl->glActiveTexture(GL_TEXTURE1);
            gl->glBindTexture(GL_TEXTURE_2D_ARRAY, lod_normalmap_arrays[lod]);

            gl->glBindBuffer(GL_ARRAY_BUFFER, lod_instance_vbos[lod]);
            gl->glBufferSubData(GL_ARRAY_BUFFER, 0, instance_count * (i32)sizeof(vast_tile_instance_data), instance_buffer);

            gl->glBindVertexArray(grid_vaos[lod]);
            gl->glDrawElementsInstanced(GL_TRIANGLES, gridIndexCounts[lod], GL_UNSIGNED_SHORT, VAST_NULL, instance_count);

            (*draw_calls)++;
        }

        gl->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gl->glDisable(GL_CULL_FACE);
    }
    VAST_PROFILER_END(tile_render);
}

#endif /* VAST_TILE_RENDERER_H */