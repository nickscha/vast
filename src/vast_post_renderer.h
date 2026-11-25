#ifndef VAST_POST_RENDERER_H
#define VAST_POST_RENDERER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)    */
#include "vast.h"                     /* Platform State                */
#include "vast_state.h"               /* Vast state struct             */
#include "vast_math_linear_algebra.h" /* Linear Algebra                */
#include "vast_opengl.h"              /* OpenGL functions              */

/* #############################################################################
 * # [SECTION] Structs and Data
 * #############################################################################
 */
typedef struct shader_post_processing
{
    shader_header header;

    i32 loc_scene_texture;
    i32 loc_iResolution;
    i32 loc_iTime;
    i32 loc_sunScreenPos;

} shader_post_processing;

static shader_post_processing post_processing_shader = {0};
static u32 post_processing_color_texture;
static u32 post_processing_vao;

static s8 *post_processing_shader_vertex_code =
    "#version 330 core\n"
    "\n"
    "out vec2 vUV;\n"
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
    "    vUV = p * 0.5 + 0.5;\n"
    "    gl_Position = vec4(p, 1.0, 1.0);\n"
    "}";

/* #############################################################################
 * # [SECTION] Post Processing Renderer
 * #############################################################################
 */
VAST_API void vast_post_render_begin(vast_platform_state *platform_state, vast_state *state)
{
    static u8 post_process_initialized = 0;

    static u32 depth_rbo;
    static i32 last_width = 0;
    static i32 last_height = 0;

    vast_opengl_api *gl = &platform_state->graphics.opengl;

    if (!post_process_initialized)
    {
        gl->glGenFramebuffers(1, &state->post_framebuffer_object_id);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, state->post_framebuffer_object_id);

        gl->glGenTextures(1, &post_processing_color_texture);
        gl->glBindTexture(GL_TEXTURE_2D, post_processing_color_texture);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (i32)platform_state->window.width, (i32)platform_state->window.height, 0, GL_RGBA, GL_FLOAT, VAST_NULL);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, post_processing_color_texture, 0);

        gl->glGenRenderbuffers(1, &depth_rbo);
        gl->glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
        gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, (i32)platform_state->window.width, (i32)platform_state->window.height);
        gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);

        gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* Load shader */
        {
            u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&platform_state->api, &state->memory_region_transient, "shaders/40_vast_post.fs");

            if (!shader_data_fragment)
            {
                return;
            }

            if (opengl_shader_load(&post_processing_shader.header, post_processing_shader_vertex_code, (s8 *)shader_data_fragment, platform_state->api.io_print, gl))
            {
                post_processing_shader.loc_scene_texture = gl->glGetUniformLocation(post_processing_shader.header.program, "u_scene_tex");
                post_processing_shader.loc_iResolution = gl->glGetUniformLocation(post_processing_shader.header.program, "iResolution");
                post_processing_shader.loc_iTime = gl->glGetUniformLocation(post_processing_shader.header.program, "iTime");
                post_processing_shader.loc_sunScreenPos = gl->glGetUniformLocation(post_processing_shader.header.program, "u_sunScreenPos");
            }
            else
            {
                platform_state->api.io_print("Cannot compile post processing shaders!\n");
            }

            gl->glGenVertexArrays(1, &post_processing_vao);
        }

        post_process_initialized = 1;
    }

    if (last_width != (i32)platform_state->window.width || last_height != (i32)platform_state->window.height)
    {

        last_width = (i32)platform_state->window.width;
        last_height = (i32)platform_state->window.height;

        gl->glBindTexture(GL_TEXTURE_2D, post_processing_color_texture);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (i32)platform_state->window.width, (i32)platform_state->window.height, 0, GL_RGBA, GL_FLOAT, VAST_NULL);

        gl->glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
        gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, (i32)platform_state->window.width, (i32)platform_state->window.height);
    }

    if (platform_state->timing.frame_count % 30 == 0)
    {
        static u8 hot_reload_initialized = 0;
        static u32 post_processing_shader_heightmap_last = 0;

        u32 post_processing_shader_heightmap_now = 0;

        if (!hot_reload_initialized)
        {
            platform_state->api.io_file_mod_signature("shaders/40_vast_post.fs", &post_processing_shader_heightmap_last);

            hot_reload_initialized = 1;
        }

        platform_state->api.io_file_mod_signature("shaders/40_vast_post.fs", &post_processing_shader_heightmap_now);

        if (post_processing_shader_heightmap_now != post_processing_shader_heightmap_last)
        {
            u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&platform_state->api, &state->memory_region_transient, "shaders/40_vast_post.fs");

            if (!shader_data_fragment)
            {
                return;
            }

            if (opengl_shader_load(&post_processing_shader.header, post_processing_shader_vertex_code, (s8 *)shader_data_fragment, platform_state->api.io_print, gl))
            {
                post_processing_shader.loc_scene_texture = gl->glGetUniformLocation(post_processing_shader.header.program, "u_scene_tex");
                post_processing_shader.loc_iResolution = gl->glGetUniformLocation(post_processing_shader.header.program, "iResolution");
                post_processing_shader.loc_iTime = gl->glGetUniformLocation(post_processing_shader.header.program, "iTime");
                post_processing_shader.loc_sunScreenPos = gl->glGetUniformLocation(post_processing_shader.header.program, "u_sunScreenPos");
            }
            else
            {
                platform_state->api.io_print("Cannot compile post processing shaders!\n");
            }
        }
    }

    gl->glBindFramebuffer(GL_FRAMEBUFFER, state->post_framebuffer_object_id);
    gl->glViewport(0, 0, (i32)platform_state->window.width, (i32)platform_state->window.height);
}

VAST_API void vast_post_render_end(vast_platform_state *platform_state, vast_state *state)
{
    vast_opengl_api *gl = &platform_state->graphics.opengl;

    vast_vec3 sun_world_pos = vast_vec3_add(state->camera.position, vast_vec3_mulf(state->sun_direction, 5000.0f));
    vast_vec4 sun_clip = vast_mat4x4_mul_vec4(state->view_projection, vast_vec4_init(sun_world_pos.x, sun_world_pos.y, sun_world_pos.z, 1.0f));
    vast_vec2 sun_screen_pos = vast_vec2_init(0.5f, 0.5f);

    if (sun_clip.w > 0.0f)
    {
        sun_screen_pos.x = (sun_clip.x / sun_clip.w) * 0.5f + 0.5f;
        sun_screen_pos.y = (sun_clip.y / sun_clip.w) * 0.5f + 0.5f;
    }
    else
    {
        sun_screen_pos.x = -10.0f;
        sun_screen_pos.y = -10.0f;
    }

    gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gl->glViewport(0, 0, (i32)platform_state->window.width, (i32)platform_state->window.height);
    gl->glDisable(GL_DEPTH_TEST);

    gl->glUseProgram(post_processing_shader.header.program);
    gl->glUniform1i(post_processing_shader.loc_scene_texture, 0);
    gl->glUniform3f(post_processing_shader.loc_iResolution, (f32)platform_state->window.width, (f32)platform_state->window.height, 1.0f);
    gl->glUniform1f(post_processing_shader.loc_iTime, (f32)platform_state->timing.time_elapsed);
    gl->glUniform2f(post_processing_shader.loc_sunScreenPos, sun_screen_pos.x, sun_screen_pos.y);

    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, post_processing_color_texture);
    gl->glBindVertexArray(post_processing_vao);
    gl->glDrawArrays(GL_TRIANGLES, 0, 3);

    state->rendering_draw_calls_post++;
}

#endif /* VAST_POST_RENDERER_H */