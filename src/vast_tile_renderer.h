#ifndef VAST_TILE_RENDERER_H
#define VAST_TILE_RENDERER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)    */
#include "vast.h"                     /* Platform State                */
#include "vast_profiler.h"            /* Profiler metrics              */
#include "vast_tiles.h"               /* Toriodal Tiling Grid logic    */
#include "vast_camera.h"              /* Camera                        */
#include "vast_geometry.h"            /* Calc. Grid with Edge LOD Mask */
#include "vast_math_linear_algebra.h" /* Linear Algebra                */
#include "vast_opengl.h"              /* OpenGL functions              */

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
    i32 loc_stamped_heightmap;
    i32 loc_stamped_heightmap_size;

} vast_tile_shader_normalmap;

typedef struct vast_tile_shader_final
{
    shader_header header;

    i32 loc_tile_offset;
    i32 loc_view_projection;
    i32 loc_is_dirty;
    i32 loc_heightmap;
    i32 loc_normalmap;
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

static s8 *vast_tile_texture_vertex_data =
    "#version 330 core\n"
    "\n"
    "out vec2 v_uv;\n"
    "\n"
    "const vec2 verts[3] = vec2[](\n"
    "    vec2(-1.0, -1.0),\n"
    "    vec2( 3.0, -1.0),\n"
    "    vec2(-1.0,  3.0)\n"
    ");\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec2 p = verts[gl_VertexID];\n"
    "    v_uv = p * 0.5 + 0.5;\n"
    "    gl_Position = vec4(p, 0.0, 1.0);\n"
    "}";

/* #############################################################################
 * # [SECTION] Tile Renderer
 * #############################################################################
 */
#define VAST_TILE_WORLD_SIZE 256.0f
#define VAST_TILE_GRID_SIZE 65
#define VAST_TILE_MAX_INDICES ((VAST_TILE_GRID_SIZE - 1) * (VAST_TILE_GRID_SIZE - 1) * 6)

VAST_API u32 vast_create_height_fbo(u32 tex)
{
    u32 fbo;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    return fbo;
}

VAST_API void vast_tile_render_texture_heightmap(vast_platform_state *state, vast_tile_shader_heightmap *tile_hm_shader, f32 tile_size, u32 fbo, u32 size, f32 tileWorldX, f32 tileWorldZ)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, (i32)size, (i32)size);

    glUseProgram(tile_hm_shader->header.program);
    glUniform3f(tile_hm_shader->loc_tile_offset, tileWorldX, tileWorldZ, 0.0f);
    glUniform1f(tile_hm_shader->loc_tile_size, (f32)tile_size);
    glUniform1f(tile_hm_shader->loc_texture_size, (f32)size);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glViewport(0, 0, (i32)state->window.width, (i32)state->window.height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

VAST_API void vast_tile_render_texture_normalmap(vast_platform_state *state, vast_tile_shader_normalmap *tile_nm_shader, f32 tile_size, u32 fbo, u32 size, f32 tileWorldX, f32 tileWorldZ, u32 heightmap_texture, f32 heightmap_texture_size)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, (i32)size, (i32)size);

    glUseProgram(tile_nm_shader->header.program);
    glUniform3f(tile_nm_shader->loc_tile_offset, tileWorldX, tileWorldZ, 0.0f);
    glUniform1f(tile_nm_shader->loc_tile_size, (f32)tile_size);
    glUniform1f(tile_nm_shader->loc_texture_size, (f32)size);
    glUniform1f(tile_nm_shader->loc_stamped_heightmap, 0);
    glUniform1f(tile_nm_shader->loc_stamped_heightmap_size, heightmap_texture_size);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmap_texture);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glViewport(0, 0, (i32)state->window.width, (i32)state->window.height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

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

    for (i = 0; i < 6; ++i)
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

/* TODO(nickscha): Temporary */
VAST_API void *vast_tile_allocate_memory(vast_platform_state *state, u32 size)
{
    /* Align to 8 bytes */
    u32 aligned_size = (size + 7u) & ~7u;

    void *ptr;

    if (state->memory.offset + aligned_size > state->memory.size)
    {
        return VAST_NULL;
    }

    ptr = (void *)(state->memory.data + state->memory.offset);
    state->memory.offset += aligned_size;

    return ptr;
}

/* TODO(nickscha): Temporary */
VAST_API void *vast_tile_read_file_to_memory(vast_platform_state *state, s8 *filename)
{
    u32 file_size = 0;
    void *data;

    if (!state->api.io_file_size(filename, &file_size) || file_size < 1)
    {
        state->api.io_print(filename);
        state->api.io_print(": Cannot optain file size!\n");
        return VAST_NULL;
    }

    data = vast_tile_allocate_memory(state, file_size);

    if (!data)
    {
        state->api.io_print(filename);
        state->api.io_print(": Cannot allocate memory for file data!\n");
        return VAST_NULL;
    }

    if (!state->api.io_file_read(filename, (u8 *)data, file_size))
    {
        state->api.io_print(filename);
        state->api.io_print(": Cannot read file into memory!\n");
        return VAST_NULL;
    }

    return data;
}

VAST_API void vast_tile_shader_heightmap_load(vast_platform_state *state, vast_tile_shader_heightmap *tiles_hm_shader)
{
    u8 *shader_data_fragment = (u8 *)vast_tile_read_file_to_memory(state, "shaders/vast_tile_hm.fs");

    if (!shader_data_fragment)
    {
        state->memory.offset = 0;
        return;
    }

    if (opengl_shader_load(&tiles_hm_shader->header, vast_tile_texture_vertex_data, (s8 *)shader_data_fragment, state->api.io_print))
    {
        tiles_hm_shader->loc_tile_offset = glGetUniformLocation(tiles_hm_shader->header.program, "u_tile_origin");
        tiles_hm_shader->loc_tile_size = glGetUniformLocation(tiles_hm_shader->header.program, "u_tile_size");
        tiles_hm_shader->loc_texture_size = glGetUniformLocation(tiles_hm_shader->header.program, "u_texture_size");
    }
    else
    {
        state->api.io_print("Cannot compile tile heightmap shaders!\n");
    }

    state->memory.offset = 0;
}

VAST_API void vast_tile_shader_normalmap_load(vast_platform_state *state, vast_tile_shader_normalmap *tiles_nm_shader)
{
    u8 *shader_data_fragment = (u8 *)vast_tile_read_file_to_memory(state, "shaders/vast_tile_nm.fs");

    if (!shader_data_fragment)
    {
        state->memory.offset = 0;
        return;
    }

    if (opengl_shader_load(&tiles_nm_shader->header, vast_tile_texture_vertex_data, (s8 *)shader_data_fragment, state->api.io_print))
    {
        tiles_nm_shader->loc_tile_offset = glGetUniformLocation(tiles_nm_shader->header.program, "u_tile_origin");
        tiles_nm_shader->loc_tile_size = glGetUniformLocation(tiles_nm_shader->header.program, "u_tile_size");
        tiles_nm_shader->loc_texture_size = glGetUniformLocation(tiles_nm_shader->header.program, "u_texture_size");
        tiles_nm_shader->loc_stamped_heightmap = glGetUniformLocation(tiles_nm_shader->header.program, "u_stamped_heightmap");
        tiles_nm_shader->loc_stamped_heightmap_size = glGetUniformLocation(tiles_nm_shader->header.program, "u_stamped_heightmap_size");
    }
    else
    {
        state->api.io_print("Cannot compile tile normal map shaders!\n");
    }

    state->memory.offset = 0;
}

VAST_API void vast_tile_shader_final_load(vast_platform_state *state, vast_tile_shader_final *tiles_shader)
{

    u8 *shader_data_vertex = (u8 *)vast_tile_read_file_to_memory(state, "shaders/vast_tiles.vs");
    u8 *shader_data_fragment = (u8 *)vast_tile_read_file_to_memory(state, "shaders/vast_tiles.fs");

    if (!shader_data_vertex || !shader_data_fragment)
    {
        state->memory.offset = 0;
        return;
    }

    if (opengl_shader_load(&tiles_shader->header, (s8 *)shader_data_vertex, (s8 *)shader_data_fragment, state->api.io_print))
    {
        tiles_shader->loc_tile_offset = glGetUniformLocation(tiles_shader->header.program, "u_tile_offset");
        tiles_shader->loc_view_projection = glGetUniformLocation(tiles_shader->header.program, "u_vp");
        tiles_shader->loc_is_dirty = glGetUniformLocation(tiles_shader->header.program, "u_is_dirty");
        tiles_shader->loc_heightmap = glGetUniformLocation(tiles_shader->header.program, "u_heightmap");
        tiles_shader->loc_normalmap = glGetUniformLocation(tiles_shader->header.program, "u_normalmap");
        tiles_shader->loc_tile_size = glGetUniformLocation(tiles_shader->header.program, "u_tile_size");
        tiles_shader->loc_grid_res = glGetUniformLocation(tiles_shader->header.program, "u_grid_res");

        tiles_shader->loc_iResolution = glGetUniformLocation(tiles_shader->header.program, "iResolution");
        tiles_shader->loc_iTime = glGetUniformLocation(tiles_shader->header.program, "iTime");
        tiles_shader->loc_iMouse = glGetUniformLocation(tiles_shader->header.program, "iMouse");
        tiles_shader->loc_sun_dir = glGetUniformLocation(tiles_shader->header.program, "sunDir");
        tiles_shader->loc_camera = glGetUniformLocation(tiles_shader->header.program, "iCamera");
        tiles_shader->loc_camera_view_dir = glGetUniformLocation(tiles_shader->header.program, "iViewDir");

        tiles_shader->loc_visualization_mode = glGetUniformLocation(tiles_shader->header.program, "visualization_mode");
    }
    else
    {
        state->api.io_print("Cannot compile tiles shaders!\n");
    }

    state->memory.offset = 0;
}

VAST_API void vast_tile_render(vast_platform_state *state, vast_camera *camera, vast_vec3 sun_dir)
{
    static u8 tiles_initialized = 0;
    static vast_tiles t = {0};
    static i32 camera_tile_x = 0;
    static i32 camera_tile_z = 0;
    static u32 visualization_mode = 0;

    static vast_tile_shader_final tiles_shader = {0};
    static vast_tile_shader_heightmap tiles_hm_shader = {0};
    static vast_tile_shader_normalmap tiles_nm_shader = {0};

#define LOD_COUNT 6
    static u32 grid_vao;
    static i32 gridIndexCounts[LOD_COUNT][16] = {0};
    static u32 grid_ibos[LOD_COUNT][16] = {0};

    static u32 tile_tex[VAST_TILES_TOTAL];
    static u32 tile_fbo[VAST_TILES_TOTAL];
    static u32 tile_normal_tex[VAST_TILES_TOTAL];
    static u32 tile_normal_fbo[VAST_TILES_TOTAL];
    static u32 tile_vao;
    static i32 tile_rendered_lod[VAST_TILES_TOTAL];

    if (!tiles_initialized)
    {
        u32 i;
        for (i = 0; i < VAST_TILES_TOTAL; ++i)
        {
            tile_rendered_lod[i] = -1; /* -1 = Uninitialized texture state */
        }

        /* Setup tiles */
        VAST_PROFILER_BEGIN(tile_init);
        vast_tiles_init(&t, camera_tile_x, camera_tile_z);
        VAST_PROFILER_END(tile_init);

        /* Setup tiles shaders */
        vast_tile_shader_heightmap_load(state, &tiles_hm_shader);
        vast_tile_shader_normalmap_load(state, &tiles_nm_shader);
        vast_tile_shader_final_load(state, &tiles_shader);

        /* Dummy tiles vertex buffer object */
        glGenVertexArrays(1, &tile_vao);

        /* Generate Grid for all LOD edge mask combinations */
        {
            u8 mask;
            u32 i;

            u16 gridIndices[VAST_TILE_MAX_INDICES];

            glGenVertexArrays(1, &grid_vao);
            glBindVertexArray(grid_vao);

            for (i = 0; i < LOD_COUNT; ++i)
            {
                i32 grid_resolution = ((VAST_TILE_GRID_SIZE - 1) / (1 << (u32)i)) + 1;

                for (mask = 0; mask < 16; ++mask)
                {
                    gridIndexCounts[i][mask] = vast_geometry_grid_lod(grid_resolution, (u32)mask, gridIndices);

                    glGenBuffers(1, &grid_ibos[i][mask]);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ibos[i][mask]);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndexCounts[i][mask] * (i32)sizeof(u16), gridIndices, GL_STATIC_DRAW);
                }
            }
        }

        tiles_initialized = 1;
    }

    /* Hot reload tile shaders */
    if (state->timing.frame_count % 15 == 0)
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
            vast_tile_shader_heightmap_load(state, &tiles_hm_shader);
            vast_tiles_init(&t, camera_tile_x, camera_tile_z); /* Regenerate terrain fully */
            tile_shader_heightmap_last = tile_shader_heightmap_now;
        }

        if (tile_shader_normalmap_now != tile_shader_normalmap_last)
        {
            vast_tile_shader_normalmap_load(state, &tiles_nm_shader);
            vast_tiles_init(&t, camera_tile_x, camera_tile_z); /* Regenerate terrain fully */
            tile_shader_normalmap_last = tile_shader_normalmap_now;
        }

        if (tile_shader_final_now != tile_shader_final_last)
        {
            vast_tile_shader_final_load(state, &tiles_shader);
            tile_shader_final_last = tile_shader_final_now;
        }

        VAST_PROFILER_END(tile_hot_reload);
    }

    /* Update camera tile based on camera world position */
    camera_tile_x = (i32)vast_floorf((camera->position.x + VAST_TILE_WORLD_SIZE * 0.5f) / VAST_TILE_WORLD_SIZE);
    camera_tile_z = (i32)vast_floorf((camera->position.z + VAST_TILE_WORLD_SIZE * 0.5f) / VAST_TILE_WORLD_SIZE);

    /* Check for new dirty tiles */
    VAST_PROFILER_BEGIN(tile_update);
    vast_tiles_update(&t, camera_tile_x, camera_tile_z);
    VAST_PROFILER_END(tile_update);

    /* Process dirty tiles */
    VAST_PROFILER_BEGIN(tile_process_dirty);
    {
        /* When no tiles calculated process everything in one frame otherwise introduce frame pacing */
        i32 updates_per_frame = t.dirty_indices_count == VAST_TILES_TOTAL ? VAST_TILES_TOTAL : 4;

        while (updates_per_frame > 0 && t.dirty_indices_count > 0)
        {
            u32 last_idx = t.dirty_indices_count - 1;
            u32 tile_idx = t.dirty_indices[last_idx];
            i32 tile_lod = t.tile_lod[tile_idx];
            u32 height_tex_size = (u32)((VAST_TILE_GRID_SIZE - 1) / (1 << (u32)tile_lod)) + 1;
            u32 normal_tex_size = 1024 / (1 << (u32)tile_lod);

            /* Height Texture */
            if (tile_tex[tile_idx] == 0)
            {
                glGenTextures(1, &tile_tex[tile_idx]);
            }

            /* TODO(nickscha): Allocate tiles texture pool once and then reuse from pool       */
            if (tile_rendered_lod[tile_idx] != tile_lod)
            {
                glBindTexture(GL_TEXTURE_2D, tile_tex[tile_idx]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, (i32)height_tex_size, (i32)height_tex_size, 0, GL_RED, GL_FLOAT, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            /* Normal Map */
            if (tile_normal_tex[tile_idx] == 0)
            {
                glGenTextures(1, &tile_normal_tex[tile_idx]);
            }

            /* TODO(nickscha): Allocate tiles texture pool once and then reuse from pool       */
            /* TODO(nickscha): Far away normal maps only need RG8 to save memory               */
            if (tile_rendered_lod[tile_idx] != tile_lod)
            {
                glBindTexture(GL_TEXTURE_2D, tile_normal_tex[tile_idx]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, (i32)normal_tex_size, (i32)normal_tex_size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            if (tile_fbo[tile_idx] == 0)
            {
                tile_fbo[tile_idx] = vast_create_height_fbo(tile_tex[tile_idx]);
            }

            if (tile_normal_fbo[tile_idx] == 0)
            {
                tile_normal_fbo[tile_idx] = vast_create_height_fbo(tile_normal_tex[tile_idx]);
            }

            glBindVertexArray(tile_vao);
            vast_tile_render_texture_heightmap(
                state, &tiles_hm_shader, VAST_TILE_WORLD_SIZE, tile_fbo[tile_idx], height_tex_size,
                (f32)t.tile_x[tile_idx], (f32)t.tile_z[tile_idx]);

            /*
            Testing only:
              - Read back the heights to CPU
              - Modify heights
              - Reupload texture with changed heights
            */
            if (t.tile_x[tile_idx] == 0 && t.tile_z[tile_idx] == 0)
            {
                f32 cpu_heights[VAST_TILE_GRID_SIZE * VAST_TILE_GRID_SIZE];
                u32 x, z;

                glBindFramebuffer(GL_FRAMEBUFFER, tile_fbo[tile_idx]);
                glReadPixels(0, 0, (i32)height_tex_size, (i32)height_tex_size, GL_RED, GL_FLOAT, cpu_heights);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                for (z = 0; z < height_tex_size; ++z)
                {
                    for (x = 0; x < height_tex_size; ++x)
                    {
                        u32 i = z * height_tex_size + x;
                        f32 h = cpu_heights[i];
                        u32 gap = 1;

                        if (z > gap && z < height_tex_size - gap && x > gap && x < height_tex_size - gap)
                        {
                            h = 0.0f;
                        }

                        cpu_heights[i] = h;
                    }
                }

                /* Apply stamps */
                {
                    f32 tile_origin_x = (f32)t.tile_x[tile_idx] * VAST_TILE_WORLD_SIZE;
                    f32 tile_origin_z = (f32)t.tile_z[tile_idx] * VAST_TILE_WORLD_SIZE;
                    f32 texel_step = VAST_TILE_WORLD_SIZE / (f32)(height_tex_size - 1);

                    typedef enum
                    {
                        STAMP_DIG,
                        STAMP_RAISE,
                        STAMP_FLATTEN
                    } vast_stamp_type;

                    typedef struct
                    {
                        vast_stamp_type type;
                        f32 world_x;
                        f32 world_z;
                        f32 radius;
                        f32 strength;      /* Depth/Height value */
                        f32 target_height; /* Used for flatten only */
                    } vast_stamp;

                    vast_stamp test_stamps[3];
                    u32 test_stamp_count = 3;

                    f32 tile_center_x = tile_origin_x + (VAST_TILE_WORLD_SIZE * 0.5f);
                    f32 tile_center_z = tile_origin_z + (VAST_TILE_WORLD_SIZE * 0.5f);
                    f32 offset = VAST_TILE_WORLD_SIZE * 0.25f;

                    test_stamps[0].type = STAMP_DIG;
                    test_stamps[0].world_x = tile_center_x - offset;
                    test_stamps[0].world_z = tile_center_z;
                    test_stamps[0].radius = VAST_TILE_WORLD_SIZE * 0.15f;
                    test_stamps[0].strength = 10.0f;

                    test_stamps[1].type = STAMP_RAISE;
                    test_stamps[1].world_x = tile_center_x + offset;
                    test_stamps[1].world_z = tile_center_z;
                    test_stamps[1].radius = VAST_TILE_WORLD_SIZE * 0.15f;
                    test_stamps[1].strength = 10.0f;

                    test_stamps[2].type = STAMP_FLATTEN;
                    test_stamps[2].world_x = tile_center_x;
                    test_stamps[2].world_z = tile_center_z;
                    test_stamps[2].radius = VAST_TILE_WORLD_SIZE * 0.18f;
                    test_stamps[2].strength = 1.0f;      /* 1.0 = hard flatten, lower = soft blend */
                    test_stamps[2].target_height = 5.0f; /* Flatten target elevation plateau */

                    for (z = 0; z < height_tex_size; ++z)
                    {
                        f32 world_z = tile_origin_z + ((f32)z * texel_step);

                        for (x = 0; x < height_tex_size; ++x)
                        {
                            f32 world_x = tile_origin_x + ((f32)x * texel_step);
                            u32 i = z * height_tex_size + x;
                            f32 h = cpu_heights[i];
                            u32 s;

                            for (s = 0; s < test_stamp_count; ++s)
                            {
                                vast_stamp *stamp = &test_stamps[s];

                                f32 dx = world_x - stamp->world_x;
                                f32 dz = world_z - stamp->world_z;
                                f32 distance_squared = dx * dx + dz * dz;

                                if (distance_squared < (stamp->radius * stamp->radius))
                                {
                                    f32 distance = vast_sqrtf(distance_squared);
                                    f32 falloff = 1.0f - (distance / stamp->radius);

                                    switch (stamp->type)
                                    {
                                    case STAMP_DIG:
                                        h -= stamp->strength * falloff;
                                        break;

                                    case STAMP_RAISE:
                                        h += stamp->strength * falloff;
                                        break;

                                    case STAMP_FLATTEN:
                                    {
                                        f32 blend_factor = falloff * stamp->strength;
                                        h = h + (stamp->target_height - h) * blend_factor;
                                    }
                                    break;
                                    }
                                }
                            }

                            cpu_heights[i] = h;
                        }
                    }
                }

                glBindTexture(GL_TEXTURE_2D, tile_tex[tile_idx]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (i32)height_tex_size, (i32)height_tex_size, GL_RED, GL_FLOAT, cpu_heights);
            }

            vast_tile_render_texture_normalmap(state, &tiles_nm_shader, VAST_TILE_WORLD_SIZE, tile_normal_fbo[tile_idx], normal_tex_size,
                                               (f32)t.tile_x[tile_idx], (f32)t.tile_z[tile_idx], tile_tex[tile_idx], (f32)height_tex_size);

            /* Mark that texture update is complete for this LOD */
            tile_rendered_lod[tile_idx] = tile_lod;

            t.dirty_indices_count--;
            updates_per_frame--;
        }
    }
    VAST_PROFILER_END(tile_process_dirty);

    /* Render Tiles */
    VAST_PROFILER_BEGIN(tile_render);
    {
        static u8 wireframe_enabled = 0;

        u32 i;

        vast_mat4x4 projection = vast_mat4x4_perspective(VAST_DEG_TO_RAD(camera->fov), (f32)state->window.width / (f32)state->window.height, 0.1f, 64000.0f);
        vast_mat4x4 view = vast_mat4x4_look_at(camera->position, vast_vec3_add(camera->position, camera->forward), camera->up);
        vast_mat4x4 mvp = vast_mat4x4_mul(projection, view);
        vast_frustum frustum = vast_frustum_extract(mvp);

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

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glUseProgram(tiles_shader.header.program);
        glUniformMatrix4fv(tiles_shader.loc_view_projection, 1, GL_FALSE, mvp.e);
        glUniform3f(tiles_shader.loc_iResolution, (f32)state->window.width, (f32)state->window.height, 1.0f);
        glUniform1f(tiles_shader.loc_iTime, (f32)state->timing.time_elapsed);
        glUniform3f(tiles_shader.loc_iMouse, (f32)state->input.mouse.x, (f32)state->input.mouse.y, 0.0f);
        glUniform3f(tiles_shader.loc_camera, camera->position.x, camera->position.y, camera->position.z);
        glUniform3f(tiles_shader.loc_camera_view_dir, camera->forward.x, camera->forward.y, camera->forward.z);
        glUniform3f(tiles_shader.loc_sun_dir, sun_dir.x, sun_dir.y, sun_dir.z);
        glUniform1f(tiles_shader.loc_visualization_mode, (f32)visualization_mode);
        glUniform1f(tiles_shader.loc_tile_size, VAST_TILE_WORLD_SIZE);
        glUniform1i(tiles_shader.loc_heightmap, 0);
        glUniform1i(tiles_shader.loc_normalmap, 1);

        glBindVertexArray(grid_vao);
        glPolygonMode(GL_FRONT_AND_BACK, wireframe_enabled ? GL_LINE : GL_FILL);

        for (i = 0; i < VAST_TILES_TOTAL; ++i)
        {
            u8 mask;
            i32 tile_lod;
            u32 grid_res;

            if (tile_rendered_lod[i] < 0)
            {
                continue;
            }

            if (!vast_tile_is_visible(&frustum, (f32)t.tile_x[i], (f32)t.tile_z[i], VAST_TILE_WORLD_SIZE, -100.0f, 600.0f))
            {
                continue;
            }

            tile_lod = tile_rendered_lod[i];
            mask = vast_get_rendered_edge_mask(&t, tile_rendered_lod, i);
            grid_res = (u32)((VAST_TILE_GRID_SIZE - 1) / (1 << (u32)tile_lod)) + 1;

            glUniform3f(tiles_shader.loc_tile_offset, (f32)t.tile_x[i], (f32)t.tile_z[i], 0.0f);
            glUniform1f(tiles_shader.loc_is_dirty, (f32)vast_tile_is_dirty(&t, i));
            glUniform1f(tiles_shader.loc_grid_res, (f32)grid_res);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tile_tex[i]);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, tile_normal_tex[i]);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ibos[tile_lod][mask]);
            glDrawElements(GL_TRIANGLES, gridIndexCounts[tile_lod][mask], GL_UNSIGNED_SHORT, (void *)0);
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);
    }
    VAST_PROFILER_END(tile_render);
}

#endif /* VAST_TILE_RENDERER_H */