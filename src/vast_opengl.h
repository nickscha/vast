#ifndef VAST_OPENGL_H
#define VAST_OPENGL_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"      /* Basic types (u8, u32, ...)    */
#include "vast_opengl_api.h" /* OpenGL function pointers      */

/* #############################################################################
 * # [SECTION] OpenGL Function Loader
 * #############################################################################
 */
typedef void *(*vast_opengl_proc)(void);
typedef vast_opengl_proc (*vast_opengl_function_loader)(s8 *function_name);

VAST_API VAST_INLINE u8 vast_opengl_load_functions(vast_opengl_function_loader load, vast_opengl_api *opengl)
{
    if (!load || !opengl)
    {
        return 0;
    }

#ifdef _MSC_VER
#pragma warning(disable : 4068)
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

    /* OpenGL 1.1 functions */
    opengl->glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
    opengl->glClear = (PFNGLCLEARPROC)load("glClear");
    opengl->glPolygonMode = (PFNGLPOLYGONMODEPROC)load("glPolygonMode");
    opengl->glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
    opengl->glEnable = (PFNGLENABLEPROC)load("glEnable");
    opengl->glDisable = (PFNGLDISABLEPROC)load("glDisable");
    opengl->glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
    opengl->glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
    opengl->glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
    opengl->glTexImage1D = (PFNGLTEXIMAGE1DPROC)load("glTexImage1D");
    opengl->glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
    opengl->glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)load("glTexSubImage2D");
    opengl->glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
    opengl->glPixelStorei = (PFNGLPIXELSTOREIPROC)load("glPixelStorei");
    opengl->glReadPixels = (PFNGLREADPIXELSPROC)load("glReadPixels");
    opengl->glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
    opengl->glGetIntegerv = (PFNGLGETINTEGERVPROC)load("glGetIntegerv");
    opengl->glCullFace = (PFNGLCULLFACEPROC)load("glCullFace");
    opengl->glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");
    opengl->glDepthMask = (PFNGLDEPTHMASKPROC)load("glDepthMask");
    opengl->glDepthFunc = (PFNGLDEPTHFUNCPROC)load("glDepthFunc");

    /* Opengl 1.1+ until current */
    opengl->glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
    opengl->glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
    opengl->glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
    opengl->glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
    opengl->glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
    opengl->glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
    opengl->glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
    opengl->glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
    opengl->glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
    opengl->glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
    opengl->glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
    opengl->glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
    opengl->glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
    opengl->glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)load("glDrawElementsInstanced");
    opengl->glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
    opengl->glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
    opengl->glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
    opengl->glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
    opengl->glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
    opengl->glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
    opengl->glUniform2f = (PFNGLUNIFORM2FPROC)load("glUniform2f");
    opengl->glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
    opengl->glUniform3i = (PFNGLUNIFORM3IPROC)load("glUniform3i");
    opengl->glUniform4f = (PFNGLUNIFORM4FPROC)load("glUniform4f");
    opengl->glUniform4fv = (PFNGLUNIFORM4FVPROC)load("glUniform4fv");
    opengl->glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)load("glUniformMatrix3fv");
    opengl->glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");
    opengl->glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");
    opengl->glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load("glGenerateMipmap");
    opengl->glTexImage3D = (PFNGLTEXIMAGE3DPROC)load("glTexImage3D");
    opengl->glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
    opengl->glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
    opengl->glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
    opengl->glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");
    opengl->glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
    opengl->glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
    opengl->glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)load("glVertexAttribIPointer");
    opengl->glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)load("glVertexAttribDivisor");
    opengl->glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCED)load("glDrawArraysInstanced");
    opengl->glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load("glGenFramebuffers");
    opengl->glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
    opengl->glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
    opengl->glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
    opengl->glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)load("glFramebufferTextureLayer");

#pragma GCC diagnostic pop

    return 1;
}

/* #############################################################################
 * # [SECTION] OpenGL Shader Compilation and Creation
 * #############################################################################
 */
typedef u8 (*vast_opengl_print)(s8 *string);

static s8 vast_opengl_shader_info_log[1024];

VAST_API i32 vast_opengl_shader_compile(s8 *shaderCode, u32 shaderType, vast_opengl_print print, vast_opengl_api *opengl)
{
    u32 shaderId = opengl->glCreateShader(shaderType);
    i32 success;

    opengl->glShaderSource(shaderId, 1, &shaderCode, 0);
    opengl->glCompileShader(shaderId);
    opengl->glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        opengl->glGetShaderInfoLog(shaderId, 1024, 0, vast_opengl_shader_info_log);

        print("[opengl] shader compilation error:\n");
        print(vast_opengl_shader_info_log);
        print("\n");

        return -1;
    }

    return (i32)shaderId;
}

VAST_API i32 vast_opengl_shader_create(u32 *shader_program, s8 *shader_vertex_code, s8 *shader_fragment_code, vast_opengl_print print, vast_opengl_api *opengl)
{
    i32 vertex_shader_id;
    i32 fragment_shader_id;
    i32 success;

    vertex_shader_id = vast_opengl_shader_compile(shader_vertex_code, GL_VERTEX_SHADER, print, opengl);

    if (vertex_shader_id == -1)
    {
        return 0;
    }

    fragment_shader_id = vast_opengl_shader_compile(shader_fragment_code, GL_FRAGMENT_SHADER, print, opengl);

    if (fragment_shader_id == -1)
    {
        return 0;
    }

    *shader_program = opengl->glCreateProgram();
    opengl->glAttachShader(*shader_program, (u32)vertex_shader_id);
    opengl->glAttachShader(*shader_program, (u32)fragment_shader_id);
    opengl->glLinkProgram(*shader_program);
    opengl->glGetProgramiv(*shader_program, GL_LINK_STATUS, &success);
    opengl->glDeleteShader((u32)vertex_shader_id);
    opengl->glDeleteShader((u32)fragment_shader_id);

    if (!success)
    {
        opengl->glGetProgramInfoLog(*shader_program, 1024, 0, vast_opengl_shader_info_log);

        print("[opengl] program creation error:\n");
        print(vast_opengl_shader_info_log);
        print("\n");

        return 0;
    }

    return 1;
}

/* #############################################################################
 * # [SECTION] OpenGL Shader API
 * #############################################################################
 */
typedef struct shader_header
{
    u32 created;
    u32 program;
    u8 had_failure;

} shader_header;

VAST_API u32 opengl_shader_load(shader_header *shader, s8 *shader_code_vertex, s8 *shader_code_fragment, vast_opengl_print print, vast_opengl_api *opengl)
{
    u32 new_program;

    if (!vast_opengl_shader_create(&new_program, shader_code_vertex, shader_code_fragment, print, opengl))
    {
        print("[opengl] compile failed, keeping old shader is present\n");
        shader->had_failure = 1;
        return 0;
    }

    shader->had_failure = 0;

    /* If there has been already a shader created delete the old one */
    if (shader->created)
    {
        opengl->glDeleteProgram(shader->program);
    }

    shader->program = new_program;

    opengl->glUseProgram(shader->program);

    shader->created = 1;

    return 1;
}

#endif /* VAST_OPENGL_H */