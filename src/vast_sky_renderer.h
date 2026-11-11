#ifndef VAST_SKY_RENDERER_H
#define VAST_SKY_RENDERER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)    */
#include "vast.h"                     /* Platform State                */
#include "vast_profiler.h"            /* Profiler metrics              */
#include "vast_camera.h"              /* Camera                        */
#include "vast_math_linear_algebra.h" /* Linear Algebra                */
#include "vast_opengl.h"              /* OpenGL functions              */

/* #############################################################################
 * # [SECTION] Structs and Data
 * #############################################################################
 */
typedef struct shader_sky
{
    shader_header header;

    i32 loc_iResolution;
    i32 loc_iTime;
    i32 loc_camera;
    i32 loc_camera_basis;
    i32 loc_sun_dir;

} shader_sky;

/* #############################################################################
 * # [SECTION] Sky Renderer
 * #############################################################################
 */
VAST_API void vast_sky_render(vast_platform_state *state, vast_camera *camera, vast_vec3 sun_dir)
{
    static u8 sky_initialized = 0;
    static shader_sky sky_shader = {0};
    static u32 sky_vao;
    static f32 camera_basis[9];

    vast_opengl_api *gl = &state->graphics.opengl;

    if (!sky_initialized)
    {
        static s8 *shader_code_vertex =
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

        u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/vast_sky.fs");

        if (!shader_data_fragment)
        {
            state->memory.offset = 0;
            return;
        }

        if (opengl_shader_load(&sky_shader.header, shader_code_vertex, (s8 *)shader_data_fragment, state->api.io_print, gl))
        {
            sky_shader.loc_iResolution = gl->glGetUniformLocation(sky_shader.header.program, "iResolution");
            sky_shader.loc_iTime = gl->glGetUniformLocation(sky_shader.header.program, "iTime");
            sky_shader.loc_camera = gl->glGetUniformLocation(sky_shader.header.program, "cameraPos");
            sky_shader.loc_camera_basis = gl->glGetUniformLocation(sky_shader.header.program, "cameraBasis");
            sky_shader.loc_sun_dir = gl->glGetUniformLocation(sky_shader.header.program, "sunDir");
        }
        else
        {
            state->api.io_print("Cannot compile shaders!\n");
        }

        state->memory.offset = 0;

        gl->glGenVertexArrays(1, &sky_vao);

        sky_initialized = 1;
    }

    VAST_PROFILER_BEGIN(sky_render);

    /* Construct camera basis */
    /* TODO(nickscha): not really needed. Use camera instead for shader */
    camera_basis[0] = camera->right.x;
    camera_basis[1] = camera->right.y;
    camera_basis[2] = camera->right.z;
    camera_basis[3] = camera->up.x;
    camera_basis[4] = camera->up.y;
    camera_basis[5] = camera->up.z;
    camera_basis[6] = camera->forward.x;
    camera_basis[7] = camera->forward.y;
    camera_basis[8] = camera->forward.z;

    /* Render */
    gl->glDisable(GL_DEPTH_TEST);
    gl->glDepthMask(GL_FALSE);

    gl->glUseProgram(sky_shader.header.program);
    gl->glBindVertexArray(sky_vao);

    gl->glUniform3f(sky_shader.loc_iResolution, (f32)state->window.width, (f32)state->window.height, 1.0f);
    gl->glUniform1f(sky_shader.loc_iTime, (f32)state->timing.time_elapsed);
    gl->glUniform3f(sky_shader.loc_sun_dir, sun_dir.x, sun_dir.y, sun_dir.z);
    gl->glUniform3f(sky_shader.loc_camera, camera->position.x, camera->position.y, camera->position.z);
    gl->glUniformMatrix3fv(sky_shader.loc_camera_basis, 1, GL_FALSE, camera_basis);

    gl->glDrawArrays(GL_TRIANGLES, 0, 3);

    gl->glDepthMask(GL_TRUE);
    gl->glEnable(GL_DEPTH_TEST);
    gl->glDepthFunc(GL_LESS);

    VAST_PROFILER_END(sky_render);
}

#endif /* VAST_SKY_RENDERER_H */