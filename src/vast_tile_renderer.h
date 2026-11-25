#ifndef VAST_TILE_RENDERER_H
#define VAST_TILE_RENDERER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)    */
#include "vast.h"                     /* Platform State                */
#include "vast_state.h"               /* Vast state struct             */
#include "vast_string_builder.h"      /* String printing               */
#include "vast_profiler.h"            /* Profiler metrics              */
#include "vast_tiles.h"               /* Toriodal Tiling Grid logic    */
#include "vast_camera.h"              /* Camera                        */
#include "vast_geometry.h"            /* Calc. Grid with Skirts        */
#include "vast_math_linear_algebra.h" /* Linear Algebra                */
#include "vast_opengl_api.h"          /* OpenGL Flags                  */
#include "vast_opengl.h"              /* TODO(nickscha): Only shader_header */

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

typedef struct vast_tile_shader_water
{
    shader_header header;

    i32 loc_tile_offset;
    i32 loc_view_projection;
    i32 loc_heightmap;
    i32 loc_normalmap;
    i32 loc_texture_array_layer;
    i32 loc_water_level;
    i32 loc_tile_size;
    i32 loc_grid_res;
    i32 loc_iTime;
    i32 loc_sun_dir;
    i32 loc_camera;
    i32 loc_texture_size;

} vast_tile_shader_water;

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
VAST_API void vast_tile_shader_heightmap_load(vast_platform_state *state, vast_state *vstate, vast_tile_shader_heightmap *tiles_hm_shader)
{
    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&state->api, &vstate->memory_region_transient, "shaders/10_vast_tile_heightmap_texture.fs");
    vast_opengl_api *gl = &state->graphics.opengl;

    if (!shader_data_fragment)
    {
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
}

VAST_API void vast_tile_shader_normalmap_load(vast_platform_state *state, vast_state *vstate, vast_tile_shader_normalmap *tiles_nm_shader)
{
    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&state->api, &vstate->memory_region_transient, "shaders/10_vast_tile_normalmap_texture.fs");
    vast_opengl_api *gl = &state->graphics.opengl;

    if (!shader_data_fragment)
    {
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
}

VAST_API void vast_tile_shader_final_load(vast_platform_state *state, vast_state *vstate, vast_tile_shader_final *tiles_shader)
{

    u8 *shader_data_vertex = (u8 *)vast_io_file_read_to_memory(&state->api, &vstate->memory_region_transient, "shaders/20_vast_tile_render.vs");
    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&state->api, &vstate->memory_region_transient, "shaders/20_vast_tile_render.fs");
    vast_opengl_api *gl = &state->graphics.opengl;

    if (!shader_data_vertex || !shader_data_fragment)
    {
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
}

VAST_API void vast_tile_shader_water_load(vast_platform_state *state, vast_state *vstate, vast_tile_shader_water *water_shader)
{
    u8 *shader_data_vertex = (u8 *)vast_io_file_read_to_memory(&state->api, &vstate->memory_region_transient, "shaders/25_vast_water.vs");
    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&state->api, &vstate->memory_region_transient, "shaders/25_vast_water.fs");
    vast_opengl_api *gl = &state->graphics.opengl;

    if (!shader_data_vertex || !shader_data_fragment)
    {
        return;
    }

    if (opengl_shader_load(&water_shader->header, (s8 *)shader_data_vertex, (s8 *)shader_data_fragment, state->api.io_print, gl))
    {
        water_shader->loc_tile_offset = gl->glGetUniformLocation(water_shader->header.program, "u_tile_offset");
        water_shader->loc_view_projection = gl->glGetUniformLocation(water_shader->header.program, "u_vp");
        water_shader->loc_heightmap = gl->glGetUniformLocation(water_shader->header.program, "u_heightmap");
        water_shader->loc_normalmap = gl->glGetUniformLocation(water_shader->header.program, "u_normalmap");
        water_shader->loc_texture_array_layer = gl->glGetUniformLocation(water_shader->header.program, "u_texture_array_layer");
        water_shader->loc_water_level = gl->glGetUniformLocation(water_shader->header.program, "u_water_level");
        water_shader->loc_tile_size = gl->glGetUniformLocation(water_shader->header.program, "u_tile_size");
        water_shader->loc_grid_res = gl->glGetUniformLocation(water_shader->header.program, "u_grid_res");
        water_shader->loc_iTime = gl->glGetUniformLocation(water_shader->header.program, "iTime");
        water_shader->loc_sun_dir = gl->glGetUniformLocation(water_shader->header.program, "sunDir");
        water_shader->loc_camera = gl->glGetUniformLocation(water_shader->header.program, "iCamera");
        water_shader->loc_texture_size = gl->glGetUniformLocation(water_shader->header.program, "u_texture_size");
    }
    else
    {
        state->api.io_print("Cannot compile water shaders!\n");
    }
}

#ifndef VAST_RELEASE
VAST_API void vast_tile_shader_hot_reload(
    vast_platform_state *state,
    vast_state *vstate,
    vast_tiles *tiles,
    vast_tile_shader_final *tiles_shader,
    vast_tile_shader_heightmap *tiles_hm_shader,
    vast_tile_shader_normalmap *tiles_nm_shader,
    f32 camera_position_x, f32 camera_position_z)
{
    static f64 hot_reload_accum = 0.0;

    hot_reload_accum += state->timing.time_delta;

    if (hot_reload_accum >= 2.0)
    {
        static u8 hot_reload_initialized = 0;

        static s8 *tile_shader_heightmap = "shaders/10_vast_tile_heightmap_texture.fs";
        static s8 *tile_shader_normalmap = "shaders/10_vast_tile_normalmap_texture.fs";
        static s8 *tile_shader_final = "shaders/20_vast_tile_render.fs";
        static u32 tile_shader_heightmap_last = 0;
        static u32 tile_shader_normalmap_last = 0;
        static u32 tile_shader_final_last = 0;

        u32 tile_shader_heightmap_now = 0;
        u32 tile_shader_normalmap_now = 0;
        u32 tile_shader_final_now = 0;

        hot_reload_accum = 0.0;

        VAST_PROFILER_BEGIN(tile_shader_hot_reload);

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
            vast_tile_shader_heightmap_load(state, vstate, tiles_hm_shader);
            vast_tiles_init(tiles, camera_position_x, camera_position_z, tiles->world_size); /* Regenerate terrain fully */
            tile_shader_heightmap_last = tile_shader_heightmap_now;
        }

        if (tile_shader_normalmap_now != tile_shader_normalmap_last)
        {
            vast_tile_shader_normalmap_load(state, vstate, tiles_nm_shader);
            vast_tiles_init(tiles, camera_position_x, camera_position_z, tiles->world_size); /* Regenerate terrain fully */
            tile_shader_normalmap_last = tile_shader_normalmap_now;
        }

        if (tile_shader_final_now != tile_shader_final_last)
        {
            vast_tile_shader_final_load(state, vstate, tiles_shader);
            tile_shader_final_last = tile_shader_final_now;
        }

        VAST_PROFILER_END(tile_shader_hot_reload);
    }
}
#endif

/* #############################################################################
 * # [SECTION] Texture Pool Slot Allocations and Ownership Tracking
 * #############################################################################
 */
/* TODO(nickscha): Not bulletproof yet */
VAST_API i32 vast_tile_aquire_texture_array_layer(
    vast_tiles *t, u32 tile_index,
    u8 *texture_layer_used,
    i32 *texture_layer_next_free,
    i32 *lod_texture_pool_size)
{
    u32 i;

    i32 lod_current = t->tile_lod_current[tile_index];
    i32 lod_required = t->tile_lod_required[tile_index];
    i32 texture_layer_current;
    i32 texture_layer_assigned;

    if (!t || !texture_layer_used || !texture_layer_next_free || !lod_texture_pool_size)
    {
        return -1;
    }

    if (lod_required < 0 || lod_required > t->tile_lod_max)
    {
        return -1;
    }

    /* Nothing changes */
    if (lod_current == lod_required && t->texture_layer[tile_index] >= 0)
    {
        return t->texture_layer[tile_index];
    }

    if (lod_current == -1)
    {
        lod_current = lod_required;
    }

    texture_layer_current = t->texture_layer[tile_index];

    /* Current LOD has texture layer assigned. Free it. */
    if (texture_layer_current >= 0 && lod_current >= 0)
    {
        texture_layer_used[lod_current * VAST_TILES_TOTAL + texture_layer_current] = 0;
        texture_layer_next_free[lod_current] = texture_layer_current;
    }

    texture_layer_assigned = texture_layer_next_free[lod_required];

    if (texture_layer_assigned < 0 || texture_layer_assigned >= lod_texture_pool_size[lod_required])
    {
        return -1;
    }

    /* Mark as used */
    texture_layer_used[lod_required * VAST_TILES_TOTAL + texture_layer_assigned] = 1;

    /* Find the next free texture layer */
    {
        i32 texture_array_layer_capacity_required = lod_texture_pool_size[lod_required];
        i32 next_free = -1;

        for (i = 0; i < (u32)texture_array_layer_capacity_required; ++i)
        {
            if (!texture_layer_used[lod_required * VAST_TILES_TOTAL + (i32)i])
            {
                next_free = (i32)i;
                break;
            }
        }

        texture_layer_next_free[lod_required] = next_free;
        t->texture_layer[tile_index] = texture_layer_assigned;
    }

    return texture_layer_assigned;
}

/* #############################################################################
 * # [SECTION] Tile Rendering
 * #############################################################################
 */
/* TODO(nickscha): texture pool capacity = tile_count + burst_headroom */
VAST_API void vast_tile_render(vast_platform_state *platform_state, vast_state *state)
{
    static u8 tiles_initialized = 0;
    static vast_tiles t = {0};

    static vast_tile_shader_heightmap tiles_hm_shader = {0};
    static vast_tile_shader_normalmap tiles_nm_shader = {0};
    static vast_tile_shader_final tiles_shader = {0};
    static vast_tile_shader_water water_shader = {0};
    static f32 global_water_level = -0.5f;

    static u32 *grid_vaos;
    static u32 *grid_vbos;
    static u32 *grid_ibos;
    static i32 *gridIndexCounts;
    static u32 *lod_instance_vbos;

    static u32 scratch_vao;
    static u32 scratch_fbo;

    /* Per-LOD Texture Array Handles */
    static u32 *lod_heightmap_arrays;
    static u32 *lod_normalmap_arrays;
    static u8 *texture_layer_used;
    static i32 *texture_layer_next_free;
    static i32 *lod_texture_pool_size;

    /* Base resolutions for height and normalmap */
    static i32 resolution_base_heightmap = 64;
    static i32 resolution_base_normalmap = 1024;

    static u32 lod_count = 0;

    vast_opengl_api *gl = &platform_state->graphics.opengl;

    if (!tiles_initialized)
    {
        /* Setup tiles */
        f32 tile_world_size = 256.0f;

        VAST_PROFILER_BEGIN(tile_init);
        vast_tiles_init(&t, state->camera.position.x, state->camera.position.z, tile_world_size);
        VAST_PROFILER_END(tile_init);

        lod_count = (u32)t.tile_lod_max + 1;

        /* Allocate permanent tile memory */
        {
            grid_vaos = (u32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(u32));
            grid_vbos = (u32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(u32));
            grid_ibos = (u32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(u32));
            gridIndexCounts = (i32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(i32));
            lod_instance_vbos = (u32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(u32));
            lod_heightmap_arrays = (u32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(u32));
            lod_normalmap_arrays = (u32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(u32));
            texture_layer_used = (u8 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * VAST_TILES_TOTAL * sizeof(u8));
            texture_layer_next_free = (i32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(i32));
            lod_texture_pool_size = (i32 *)vast_memory_allocate(&state->memory_region_permanent, lod_count * sizeof(i32));
        }

        /* Calculate texture pool size */
        {
            u32 lod;

            for (lod = 0; lod < lod_count; ++lod)
            {
                u32 texture_pool_size = vast_tile_calc_pool_size(&t, lod);

                s8 tmp[128];
                vast_sb sb = {0};
                sb.size = sizeof(tmp);
                sb.buffer = tmp;

                vast_sb_s8(&sb, "lod: ");
                vast_sb_i32(&sb, (i32)lod);
                vast_sb_s8(&sb, ", texture_pool_size: ");
                vast_sb_i32(&sb, (i32)texture_pool_size);
                vast_sb_s8(&sb, "\n");

                platform_state->api.io_print(sb.buffer);

                lod_texture_pool_size[lod] = (i32)texture_pool_size;
            }
        }

        /* Setup tiles texture pools */
        {
            u32 lod;

            for (lod = 0; lod < lod_count; ++lod)
            {
                i32 resolution_heightmap = resolution_base_heightmap / (1 << lod) + 1; /* +1 because of skirts */
                i32 resolution_normalmap = resolution_base_normalmap / (1 << lod);
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
        vast_tile_shader_heightmap_load(platform_state, state, &tiles_hm_shader);
        vast_tile_shader_normalmap_load(platform_state, state, &tiles_nm_shader);
        vast_tile_shader_final_load(platform_state, state, &tiles_shader);
        vast_tile_shader_water_load(platform_state, state, &water_shader);

        /* Dummy vertex and framebuffer object */
        gl->glGenVertexArrays(1, &scratch_vao);
        gl->glGenFramebuffers(1, &scratch_fbo);

        /* Generate Grid with Skirts for all LOD */
        {
            u32 lod;

            /* Allocate enough memory for vertices and indices */
            u32 vertices_count = ((u32)resolution_base_heightmap + 1) * ((u32)resolution_base_heightmap + 1) + 600;
            u32 indices_count = ((u32)resolution_base_heightmap + 1) * ((u32)resolution_base_heightmap + 1) * 6 + 2400;

            vast_geometry_vertex_skirt *vertices = (vast_geometry_vertex_skirt *)vast_memory_allocate(&state->memory_region_transient, sizeof(vast_geometry_vertex_skirt) * vertices_count);
            u16 *indices = (u16 *)vast_memory_allocate(&state->memory_region_transient, sizeof(u16) * indices_count);

            for (lod = 0; lod < lod_count; ++lod)
            {
                i32 grid_res = (resolution_base_heightmap / (1 << lod)) + 1;

                gridIndexCounts[lod] = vast_geometry_grid_skirt(grid_res, vertices, indices);

                gl->glGenVertexArrays(1, &grid_vaos[lod]);
                gl->glBindVertexArray(grid_vaos[lod]);

                /* 1. Vertex Buffer */
                gl->glGenBuffers(1, &grid_vbos[lod]);
                gl->glBindBuffer(GL_ARRAY_BUFFER, grid_vbos[lod]);
                gl->glBufferData(GL_ARRAY_BUFFER, (i32)sizeof(vast_geometry_vertex_skirt) * (i32)vertices_count, vertices, GL_STATIC_DRAW);

                /* 2. Index Buffer */
                gl->glGenBuffers(1, &grid_ibos[lod]);
                gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ibos[lod]);
                gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndexCounts[lod] * (i32)sizeof(u16), indices, GL_STATIC_DRAW);

                /* Attribute 0: UV (vec2) - Per-vertex */
                gl->glEnableVertexAttribArray(0);
                gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vast_geometry_vertex_skirt), VAST_NULL);

                /* Attribute 1: Is Skirt Flag (float) - Per-vertex */
                gl->glEnableVertexAttribArray(1);
                gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(vast_geometry_vertex_skirt), (void *)(sizeof(f32) * 2));

                /* 3. Instance VBO */
                gl->glGenBuffers(1, &lod_instance_vbos[lod]);
                gl->glBindBuffer(GL_ARRAY_BUFFER, lod_instance_vbos[lod]);
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
    vast_tile_shader_hot_reload(platform_state, state, &t, &tiles_shader, &tiles_hm_shader, &tiles_nm_shader, state->camera.position.x, state->camera.position.z);
#endif

    /* Test camera movement tile change */
    if (platform_state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_I] && !platform_state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_I])
    {
        static u8 camera_sim_enabled = 0;

        if (!camera_sim_enabled)
        {
            state->camera.position = vast_vec3_init(-128.0f, 146.0f, 51.0f);
            state->camera.forward = vast_vec3_init(1.0f, 0.0f, 0.0f);
            state->camera.right = vast_vec3_init(0.0f, 0.0f, 1.0f);
            state->camera.yaw = 2.8f;
            state->camera.fov = 29.0f;
            camera_sim_enabled = 1;
        }
        else
        {
            state->camera.position.x = state->camera.position.x < -128.0f ? -128.0f : -129.0f;
        }
    }

    /* Check for new dirty tiles */
    VAST_PROFILER_BEGIN(tile_update);
    vast_tiles_update(&t, state->camera.position.x, state->camera.position.z);
    VAST_PROFILER_END(tile_update);

    /* Process dirty tiles */
    if (t.dirty_indices_count > 0)
    {
        /* When no tiles calculated process everything in one frame otherwise introduce frame pacing */
        i32 updates_per_frame = t.dirty_indices_count == VAST_TILES_TOTAL ? VAST_TILES_TOTAL : 6;

        VAST_PROFILER_BEGIN(tile_process_dirty);

        while (updates_per_frame > 0 && t.dirty_indices_count > 0)
        {
            u32 last_idx = t.dirty_indices_count - 1;
            u32 tile_idx = t.dirty_indices[last_idx];

            i8 tile_lod_required = t.tile_lod_required[tile_idx];

            i32 height_tex_size = resolution_base_heightmap / (1 << tile_lod_required) + 1; /* +1 because of skirts */
            i32 normal_tex_size = resolution_base_normalmap / (1 << tile_lod_required);

            i32 texture_array_layer = vast_tile_aquire_texture_array_layer(&t, tile_idx, texture_layer_used, texture_layer_next_free, lod_texture_pool_size);

            if (texture_array_layer == -1)
            {
                s8 tmp[128];
                vast_sb sb = {0};
                sb.size = sizeof(tmp);
                sb.buffer = tmp;

                vast_sb_s8(&sb, "tile_index: ");
                vast_sb_i32(&sb, (i32)tile_idx);
                vast_sb_s8(&sb, "\n");

                vast_sb_s8(&sb, "tile_lod_required: ");
                vast_sb_i32(&sb, (i32)tile_lod_required);
                vast_sb_s8(&sb, "\n");

                platform_state->api.io_print("[ERROR] no next free texutre_array_layer left!\n");
                platform_state->api.io_print(sb.buffer);
            }

            gl->glBindVertexArray(scratch_vao);
            gl->glBindFramebuffer(GL_FRAMEBUFFER, scratch_fbo);

            /* Render Height Map to Framebuffer */
            VAST_PROFILER_BEGIN(tile_render_heightmap);
            {
                gl->glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, lod_heightmap_arrays[(u32)tile_lod_required], 0, texture_array_layer);
                gl->glViewport(0, 0, height_tex_size, height_tex_size);

                gl->glUseProgram(tiles_hm_shader.header.program);
                gl->glUniform2f(tiles_hm_shader.loc_tile_offset, (f32)t.tile_x[tile_idx], (f32)t.tile_z[tile_idx]);
                gl->glUniform1f(tiles_hm_shader.loc_texture_size, (f32)height_tex_size);

                /* TODO(nickscha): only need to be set once and not every frame! */
                gl->glUniform1f(tiles_hm_shader.loc_tile_size, t.world_size);

                gl->glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            VAST_PROFILER_END(tile_render_heightmap);

            /* Render Normal Map to Framebuffer */
            VAST_PROFILER_BEGIN(tile_render_normalmap);
            {
                gl->glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, lod_normalmap_arrays[(u32)tile_lod_required], 0, texture_array_layer);
                gl->glViewport(0, 0, normal_tex_size, normal_tex_size);

                gl->glUseProgram(tiles_nm_shader.header.program);
                gl->glUniform2f(tiles_nm_shader.loc_tile_offset, (f32)t.tile_x[tile_idx], (f32)t.tile_z[tile_idx]);
                gl->glUniform1f(tiles_nm_shader.loc_texture_size, (f32)normal_tex_size);

                /* TODO(nickscha): only need to be set once and not every frame! */
                gl->glUniform1f(tiles_nm_shader.loc_tile_size, t.world_size);

                gl->glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            VAST_PROFILER_END(tile_render_normalmap);

            /* Restore Viewport state */
            gl->glViewport(0, 0, (i32)platform_state->window.width, (i32)platform_state->window.height);
            gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

            t.tile_lod_current[tile_idx] = tile_lod_required;
            t.dirty_indices_count--;
            updates_per_frame--;
        }

        VAST_PROFILER_END(tile_process_dirty);
    }

    /* Render Tiles */
    VAST_PROFILER_BEGIN(tile_render);
    {
        static u8 wireframe_enabled = 0;
        static u32 visualization_mode = 0;
        static vast_tile_instance_data instance_buffer[VAST_TILES_TOTAL];

        u32 lod;

        if (platform_state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_TAB] && !platform_state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_TAB])
        {
            wireframe_enabled = !wireframe_enabled;
        }

        if (platform_state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_V] && !platform_state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_V])
        {
            visualization_mode++;

            if (visualization_mode > 4)
            {
                visualization_mode = 0;
            }
        }

        if (state->rendering_enabled_post)
        {
            gl->glBindFramebuffer(GL_FRAMEBUFFER, state->post_framebuffer_object_id);
        }

        gl->glEnable(GL_CULL_FACE);
        gl->glCullFace(GL_BACK);

        gl->glEnable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_TRUE);
        gl->glDepthFunc(GL_LESS);

        gl->glUseProgram(tiles_shader.header.program);
        gl->glUniformMatrix4fv(tiles_shader.loc_view_projection, 1, GL_FALSE, state->view_projection.e);
        gl->glUniform3f(tiles_shader.loc_iResolution, (f32)platform_state->window.width, (f32)platform_state->window.height, 1.0f);
        gl->glUniform1f(tiles_shader.loc_iTime, (f32)platform_state->timing.time_elapsed);
        gl->glUniform3f(tiles_shader.loc_iMouse, (f32)platform_state->input.mouse.x, (f32)platform_state->input.mouse.y, 0.0f);
        gl->glUniform3f(tiles_shader.loc_camera, state->camera.position.x, state->camera.position.y, state->camera.position.z);
        gl->glUniform3f(tiles_shader.loc_camera_view_dir, state->camera.forward.x, state->camera.forward.y, state->camera.forward.z);
        gl->glUniform3f(tiles_shader.loc_sun_dir, state->sun_direction.x, state->sun_direction.y, state->sun_direction.z);
        gl->glUniform1f(tiles_shader.loc_visualization_mode, (f32)visualization_mode);

        gl->glUniform1f(tiles_shader.loc_tile_size, t.world_size);
        gl->glUniform1i(tiles_shader.loc_heightmap, 0);
        gl->glUniform1i(tiles_shader.loc_normalmap, 1);

        gl->glPolygonMode(GL_FRONT_AND_BACK, wireframe_enabled ? GL_LINE : GL_FILL);

        for (lod = 0; lod < lod_count; ++lod)
        {

            i32 instance_count = 0;
            u32 tile_index;
            i32 grid_res = (resolution_base_heightmap / (1 << lod)) + 1;

            for (tile_index = 0; tile_index < VAST_TILES_TOTAL; ++tile_index)
            {
                i32 tile_lod_current = t.tile_lod_current[tile_index];
                i32 texture_array_layer = t.texture_layer[tile_index];

                if (tile_lod_current != (i32)lod || texture_array_layer < 0)
                {
                    continue;
                }

                if (!vast_tile_is_visible(&t, &state->view_frustum, tile_index, -200.0f, 600.0f))
                {
                    /* Stop LOD geomorphing since the tile is not visible */
                    t.tile_lod_transition_factor[tile_index] = 1.0f;
                    continue;
                }

                /* LOD Geomorphing: Fine to Coarse transition when moving backwards */
                if (vast_tile_is_on_lod_outer_edge(&t, tile_index))
                {
                    f32 threshold = 0.1f;

                    /* 1.0f at boundary - 0.0f at edge */
                    f32 camera_tile_factor = vast_tile_calculate_camera_backface_edge_factor(
                        &t,
                        state->camera.position.x, state->camera.position.z,
                        state->camera.forward.x, state->camera.forward.z,
                        threshold);

                    (void)camera_tile_factor;

                    /*tile_transition_timer[tile_index] = camera_tile_factor;*/
                }

                instance_buffer[instance_count].offset_x = (f32)t.tile_x[tile_index];
                instance_buffer[instance_count].offset_z = (f32)t.tile_z[tile_index];
                instance_buffer[instance_count].texture_array_layer = (f32)texture_array_layer;
                instance_buffer[instance_count].is_dirty = (f32)vast_tile_is_dirty(&t, tile_index);
                instance_buffer[instance_count].transition_factor = t.tile_lod_transition_factor[tile_index];
                instance_count++;
            }

            if (instance_count == 0)
            {
                continue;
            }

            if (!state->rendering_enabled_terrain)
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

            state->rendering_draw_calls_terrain++;
        }

        gl->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gl->glDisable(GL_CULL_FACE);

        /* Render Water Pass */
        VAST_PROFILER_BEGIN(water_render);
        {
            gl->glUseProgram(water_shader.header.program);
            gl->glUniformMatrix4fv(water_shader.loc_view_projection, 1, GL_FALSE, state->view_projection.e);
            gl->glUniform1f(water_shader.loc_water_level, global_water_level);
            gl->glUniform1f(water_shader.loc_tile_size, t.world_size);
            gl->glUniform1f(water_shader.loc_iTime, (f32)platform_state->timing.time_elapsed);
            gl->glUniform3f(water_shader.loc_sun_dir, state->sun_direction.x, state->sun_direction.y, state->sun_direction.z);
            gl->glUniform3f(water_shader.loc_camera, state->camera.position.x, state->camera.position.y, state->camera.position.z);
            gl->glUniform1i(water_shader.loc_heightmap, 0);

            /* Enable alpha blending for transparent water */
            gl->glEnable(GL_BLEND);
            gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            /* Keep depth testing enabled so terrain occludes water correctly,
               but disable depth writing so overlapping water waves blend smoothly */
            gl->glDepthMask(GL_FALSE);

            for (lod = 0; lod < lod_count; ++lod)
            {
                i32 instance_count = 0;
                u32 tile_index;
                i32 grid_res = (resolution_base_heightmap / (1 << lod)) + 1;
                i32 height_tex_size = (resolution_base_heightmap / (1 << (u32)lod)) + 1; /* Match your terrain height size calculation */

                gl->glUniform1f(water_shader.loc_grid_res, (f32)grid_res);
                gl->glUniform1f(water_shader.loc_texture_size, (f32)height_tex_size);

                for (tile_index = 0; tile_index < VAST_TILES_TOTAL; ++tile_index)
                {
                    i32 tile_lod_current = t.tile_lod_current[tile_index];
                    i32 texture_array_layer = t.texture_layer[tile_index];

                    if (tile_lod_current != (i32)lod || texture_array_layer < 0)
                    {
                        continue;
                    }

                    if (!vast_tile_is_visible(&t, &state->view_frustum, tile_index, -200.0f, 600.0f))
                    {
                        continue;
                    }

                    instance_buffer[instance_count].offset_x = (f32)t.tile_x[tile_index];
                    instance_buffer[instance_count].offset_z = (f32)t.tile_z[tile_index];
                    instance_buffer[instance_count].texture_array_layer = (f32)texture_array_layer;
                    instance_count++;
                }

                if (instance_count == 0)
                {
                    continue;
                }

                if (!state->rendering_enabled_water)
                {
                    continue;
                }

                gl->glUniform1f(water_shader.loc_grid_res, (f32)grid_res);

                gl->glActiveTexture(GL_TEXTURE0);
                gl->glBindTexture(GL_TEXTURE_2D_ARRAY, lod_heightmap_arrays[lod]);

                gl->glBindBuffer(GL_ARRAY_BUFFER, lod_instance_vbos[lod]);
                gl->glBufferSubData(GL_ARRAY_BUFFER, 0, instance_count * (i32)sizeof(vast_tile_instance_data), instance_buffer);

                gl->glBindVertexArray(grid_vaos[lod]);
                gl->glDrawElementsInstanced(GL_TRIANGLES, gridIndexCounts[lod], GL_UNSIGNED_SHORT, VAST_NULL, instance_count);

                state->rendering_draw_calls_water++;
            }

            /* Restore OpenGL state */
            gl->glDepthMask(GL_TRUE);
            gl->glDisable(GL_BLEND);
        }
        VAST_PROFILER_END(water_render);

        if (state->rendering_enabled_post)
        {
            gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    VAST_PROFILER_END(tile_render);
}

#endif /* VAST_TILE_RENDERER_H */