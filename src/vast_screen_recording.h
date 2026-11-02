#ifndef VAST_SCREEN_RECORDING_H
#define VAST_SCREEN_RECORDING_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"          /* Basic types (u8, u32, ...)  */
#include "vast.h"                /* Platform State              */
#include "vast_font.h"           /* Font Bitmap data            */
#include "vast_string_builder.h" /* String parsing              */
#include "vast_opengl.h"         /* OpenGL functions            */

/* #############################################################################
 * # [SECTION] Structs and Data
 * #############################################################################
 */
typedef struct vast_screen_recording_shader
{
    shader_header header;

    i32 loc_iResolution;
    i32 loc_iTime;

} vast_screen_recording_shader;

static s8 *vast_screen_recording_shader_vertex =
    "#version 330 core\n"
    "vec2 quad[3]=vec2[3]("
    "vec2(-1.0,-1.0),"
    "vec2(3.0,-1.0),"
    "vec2(-1.0,3.0)"
    ");"
    "void main(){gl_Position=vec4(quad[gl_VertexID],0.0,1.0);}";

static s8 *vast_screen_recording_shader_fragment =
    "#version 330 core\n"
    "out vec4 FragColor;"
    "uniform float iTime;"
    "uniform vec3 iRes;"
    "void main()"
    "{"
    "float r=10.0;"
    "float m=10.0;"
    "vec2 p=gl_FragCoord.xy;"
    "vec2 c=vec2(iRes.x-r-m, r+m);"
    "float d=length(p-c);"
    "float blink=0.5+0.5*sin(iTime*12.0);"
    "float b=4.0;"
    "if ((d<=r&&blink>0.5)||(p.x<b||p.y<b||p.x>=iRes.x-b||p.y>=iRes.y-b))"
    " FragColor=vec4(1.0,0.0,0.0,blink);"
    "else"
    " discard;"
    "}";

/* #############################################################################
 * # [SECTION] UI Setup & Rendering
 * #############################################################################
 */
VAST_API void vast_screen_recording_render(
    vast_platform_state *state,
    vast_opengl_print print,
    u8 *framebuffer)
{
    static u8 screen_recording_initialized = 0;
    static vast_screen_recording_shader recording_shader = {0};
    static u32 screeen_recording_vao;

    if (!screen_recording_initialized)
    {
        /* Generate a dummy vao with no buffer */
        glGenVertexArrays(1, &screeen_recording_vao);
        glBindVertexArray(screeen_recording_vao);

        /* Load recording shader */
        if (opengl_shader_load(&recording_shader.header, vast_screen_recording_shader_vertex, vast_screen_recording_shader_fragment, print))
        {
            recording_shader.loc_iResolution = glGetUniformLocation(recording_shader.header.program, "iRes");
            recording_shader.loc_iTime = glGetUniformLocation(recording_shader.header.program, "iTime");
        }

        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        screen_recording_initialized = 1;
    }

    /* Readback current image pixels to CPU */
    glReadPixels(0, 0, (i32)state->window.width, (i32)state->window.height, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

    /* Render recording indicator */
    glUseProgram(recording_shader.header.program);
    glUniform3f(recording_shader.loc_iResolution, (f32)state->window.width, (f32)state->window.height, 1.0f);
    glUniform1f(recording_shader.loc_iTime, (f32)state->timing.time_elapsed);
    glBindVertexArray(screeen_recording_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

#endif /* VAST_SCREEN_RECORDING_H */