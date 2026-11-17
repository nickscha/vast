#ifndef VAST_FONT_RENDERER_H
#define VAST_FONT_RENDERER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)    */
#include "vast.h"                     /* Platform State                */
#include "vast_profiler.h"            /* Profiler metrics              */
#include "vast_math_linear_algebra.h" /* Linear Algebra                */
#include "vast_opengl.h"              /* OpenGL functions              */

/* #############################################################################
 * # [SECTION] Structs and Data
 * #############################################################################
 */
typedef struct shader_font
{
    shader_header header;

    i32 loc_iResolution;
    i32 loc_iTime;
    i32 loc_texture_font;

} shader_font;

typedef struct vast_font_instance_data
{
    f32 pos_x;       /* Screen X in pixels (or NDC) */
    f32 pos_y;       /* Screen Y in pixels */
    f32 size_x;      /* Glyph width in pixels */
    f32 size_y;      /* Glyph height in pixels */
    u32 color;       /* Packed RGBA8 (0xAABBGGRR) */
    u32 glyph_index; /* 0..255 index into the 16x16 atlas */

} vast_font_instance_data;

/* #############################################################################
 * # [SECTION] Font Data creation
 * #############################################################################
 */
VAST_API u32 vast_font_update(vast_platform_state *state, vast_font_instance_data *instance_data, u32 instance_data_size)
{
    u32 instance_data_count = 0;

    s8 *text = "Vast Engine - v0.1";
    u32 i = 0;
    f32 glyph_size = 16.0f;
    f32 cursor_x = 10.0f;
    f32 cursor_y = (f32)state->window.height - glyph_size - 10.0f;

    while (text[i] != '\0' && instance_data_count < instance_data_size)
    {
        vast_font_instance_data glyph = {0};
        glyph.pos_x = (f32)(i32)(cursor_x + 0.5f);
        glyph.pos_y = (f32)(i32)(cursor_y + 0.5f);
        glyph.size_x = (f32)(i32)(glyph_size + 0.5f);
        glyph.size_y = (f32)(i32)(glyph_size + 0.5f);
        glyph.color = 0xFFFFFFFF; /* White (0xAABBGGRR) */
        glyph.glyph_index = (u32)(u8)text[i];

        instance_data[instance_data_count++] = glyph;

        cursor_x += glyph_size * 0.55f;
        i++;
    }

    (void)state;

    return instance_data_count;
}

/* #############################################################################
 * # [SECTION] Font Renderer
 * #############################################################################
 */
VAST_API void vast_font_render(vast_platform_state *state, u32 fbo, u8 post_process_enabled)
{
    static u8 font_initialized = 0;

    static shader_font font_shader = {0};
    static u32 font_vao = 0;
    static u32 font_vbo = 0;
    static u32 font_texture = 0;
    static u32 font_texture_width = 1024;
    static u32 font_texture_height = 1024;

#define INSTANCE_DATA_COUNT 512
    static vast_font_instance_data instance_data[INSTANCE_DATA_COUNT];
    static u32 instance_data_count = 0;

    vast_opengl_api *gl = &state->graphics.opengl;

    (void)instance_data;
    (void)instance_data_count;

    if (!font_initialized)
    {
        u8 *shader_data_vertex = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/vast_font.vs");
        u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/vast_font.fs");
        u8 *font_data = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "assets/font.raw");

        if (!shader_data_vertex || !shader_data_fragment || !font_data)
        {
            state->memory.offset = 0;
            return;
        }

        /* Load Shader */
        if (opengl_shader_load(&font_shader.header, (s8 *)shader_data_vertex, (s8 *)shader_data_fragment, state->api.io_print, gl))
        {
            font_shader.loc_iResolution = gl->glGetUniformLocation(font_shader.header.program, "iResolution");
            font_shader.loc_iTime = gl->glGetUniformLocation(font_shader.header.program, "iTime");
            font_shader.loc_texture_font = gl->glGetUniformLocation(font_shader.header.program, "tex_font");
        }
        else
        {
            state->api.io_print("Cannot compile font shaders!\n");
        }

        /* Load Font Texture */
        gl->glGenTextures(1, &font_texture);
        gl->glBindTexture(GL_TEXTURE_2D, font_texture);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (i32)font_texture_width, (i32)font_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, font_data);
        gl->glGenerateMipmap(GL_TEXTURE_2D);
        gl->glActiveTexture(GL_TEXTURE0);

        state->memory.offset = 0;

        /* Setup Instanced VAO + VBO */
        gl->glGenVertexArrays(1, &font_vao);
        gl->glBindVertexArray(font_vao);

        gl->glGenBuffers(1, &font_vbo);
        gl->glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(instance_data), 0, GL_DYNAMIC_DRAW);

        /* location 0: vec4 a_rect (pos_x, pos_y, size_x, size_y) */
        gl->glEnableVertexAttribArray(0);
        gl->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vast_font_instance_data), (void *)0);
        gl->glVertexAttribDivisor(0, 1);

        /* location 1: uint a_color (packed RGBA8) */
        gl->glEnableVertexAttribArray(1);
        gl->glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vast_font_instance_data), (void *)(4 * sizeof(f32)));
        gl->glVertexAttribDivisor(1, 1);

        /* location 2: uint a_glyph_index (0..255) */
        gl->glEnableVertexAttribArray(2);
        gl->glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(vast_font_instance_data), (void *)(4 * sizeof(f32) + sizeof(u32)));
        gl->glVertexAttribDivisor(2, 1);

        gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        gl->glBindVertexArray(0);

        font_initialized = 1;
    }

    /* Update */
    VAST_PROFILER_BEGIN(font_update);
    {
        instance_data_count = vast_font_update(state, instance_data, INSTANCE_DATA_COUNT);
    }
    VAST_PROFILER_END(font_update);

    if (instance_data_count == 0)
    {
        return;
    }

    /* Render */
    VAST_PROFILER_BEGIN(font_render);
    {
        if (post_process_enabled)
        {
            gl->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        }

        gl->glDisable(GL_DEPTH_TEST);
        gl->glEnable(GL_BLEND);
        gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        gl->glUseProgram(font_shader.header.program);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, font_texture);

        gl->glUniform3f(font_shader.loc_iResolution, (f32)state->window.width, (f32)state->window.height, 1.0f);
        gl->glUniform1f(font_shader.loc_iTime, (f32)state->timing.time_elapsed);
        gl->glUniform1i(font_shader.loc_texture_font, 0);

        /* Upload dynamic glyph instances */
        gl->glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
        gl->glBufferSubData(GL_ARRAY_BUFFER, 0, (i32)instance_data_count * (i32)sizeof(vast_font_instance_data), instance_data);
        gl->glBindBuffer(GL_ARRAY_BUFFER, 0);

        /* Draw 4-vertex triangle strip per glyph instance */
        gl->glBindVertexArray(font_vao);
        gl->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instance_data_count);
        gl->glBindVertexArray(0);

        gl->glDisable(GL_BLEND);
        gl->glEnable(GL_DEPTH_TEST);

        if (post_process_enabled)
        {
            gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    VAST_PROFILER_END(font_render);
}

#endif /* VAST_FONT_RENDERER_H */