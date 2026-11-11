#ifndef VAST_OPENGL_API_H
#define VAST_OPENGL_API_H

#include "vast_types.h"

/* #############################################################################
 * # [SECTION] OpenGL API
 * #############################################################################
 */
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_UNSIGNED_SHORT 0x1403
#define GL_FLOAT 0x1406
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_FAN 0x0006
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#define GL_MULTISAMPLE 0x809D
#define GL_COMPILE_STATUS 0x8B81
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_LINK_STATUS 0x8B82
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_STREAM_DRAW 0x88E0
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_INT 0x1405
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_TEXTURE_3D 0x806F
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_REPEAT 0x2901
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_RG 0x8227
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_RG8 0x822B
#define GL_RGB8 0x8051
#define GL_RGBA8 0x8058
#define GL_RGB16F 0x881B
#define GL_RED 0x1903
#define GL_RED_INTEGER 0x8D94
#define GL_R8 0x8229
#define GL_R8UI 0x8232
#define GL_R8_SNORM 0x8F94
#define GL_R16UI 0x8234
#define GL_R32F 0x822E
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DEPTH_TEST 0x0B71
#define GL_LEQUAL 0x0203
#define GL_LESS 0x0201
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_BACK 0x0405
#define GL_FRONT_AND_BACK 0x0408
#define GL_CULL_FACE 0x0B44
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_NUM_EXTENSIONS 0x821D
#define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
#define GL_CONTEXT_PROFILE_MASK 0x9126
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001

/* OpenGL 1.1 functions */
typedef void (*PFNGLCLEARCOLORPROC)(f32 red, f32 green, f32 blue, f32 alpha);
typedef void (*PFNGLCLEARPROC)(u32 mask);
typedef void (*PFNGLPOLYGONMODEPROC)(u32 face, u32 mode);
typedef void (*PFNGLVIEWPORTPROC)(i32 x, i32 y, i32 width, i32 height);
typedef void (*PFNGLENABLEPROC)(u32 cap);
typedef void (*PFNGLDISABLEPROC)(u32 cap);
typedef u8 *(*PFNGLGETSTRINGPROC)(u32 name);
typedef void (*PFNGLGENTEXTURESPROC)(i32 n, u32 *textures);
typedef void (*PFNGLBINDTEXTUREPROC)(u32 target, u32 texture);
typedef void (*PFNGLTEXIMAGE1DPROC)(u32 target, i32 level, i32 internalformat, i32 width, i32 border, i32 format, u32 type, const void *pixels);
typedef void (*PFNGLTEXIMAGE2DPROC)(u32 target, i32 level, i32 internalformat, i32 width, i32 height, i32 border, i32 format, u32 type, const void *pixels);
typedef void (*PFNGLTEXSUBIMAGE2DPROC)(u32 target, i32 level, i32 xoffset, i32 yoffset, i32 width, i32 height, i32 format, u32 type, const void *pixels);
typedef void (*PFNGLTEXPARAMETERIPROC)(u32 target, u32 pname, i32 param);
typedef void (*PFNGLPIXELSTOREIPROC)(u32 pname, i32 param);
typedef void (*PFNGLREADPIXELSPROC)(i32 x, i32 y, i32 width, i32 height, i32 format, i32 type, void *pixels);
typedef void (*PFNGLBLENDFUNCPROC)(u32 sfactor, u32 dfactor);
typedef void (*PFNGLGETINTEGERVPROC)(u32 pname, i32 *params);
typedef void (*PFNGLCULLFACEPROC)(u32 mode);
typedef void (*PFNGLDRAWELEMENTSPROC)(u32 mode, i32 count, i32 type, void *indices);
typedef void (*PFNGLDEPTHMASKPROC)(u8 flag);
typedef void (*PFNGLDEPTHFUNCPROC)(i32 func);

/* Opengl 1.1+ until current */
typedef u32 (*PFNGLCREATESHADERPROC)(u32 shaderType);
typedef u32 (*PFNGLCREATEPROGRAMPROC)(void);
typedef void (*PFNGLDELETEPROGRAMPROC)(u32 program);
typedef void (*PFNGLATTACHSHADERPROC)(u32 program, u32 shader);
typedef void (*PFNGLSHADERSOURCEPROC)(u32 shader, i32 count, s8 **string, i32 *length);
typedef void (*PFNGLCOMPILESHADERPROC)(u32 shader);
typedef void (*PFNGLGETSHADERIVPROC)(u32 shader, u32 pname, i32 *params);
typedef void (*PFNGLGETSHADERINFOLOGPROC)(u32 shader, i32 maxLength, i32 *length, s8 *infoLog);
typedef void (*PFNGLLINKPROGRAMPROC)(u32 program);
typedef void (*PFNGLGETPROGRAMIVPROC)(u32 program, u32 pname, i32 *params);
typedef void (*PFNGLGETPROGRAMINFOLOGPROC)(u32 program, i32 maxLength, i32 *length, s8 *infoLog);
typedef void (*PFNGLDELETESHADERPROC)(u32 shader);
typedef void (*PFNGLDRAWARRAYSPROC)(u32 mode, i32 first, i32 count);
typedef void (*PFNGLDRAWELEMENTSINSTANCEDPROC)(u32 mode, i32 count, u32 type, void *indices, i32 primcount);
typedef void (*PFNGLUSEPROGRAMPROC)(u32 program);
typedef void (*PFNGLGENVERTEXARRAYSPROC)(i32 n, u32 *arrays);
typedef void (*PFNGLBINDVERTEXARRAYPROC)(u32 array);
typedef i32 (*PFNGLGETUNIFORMLOCATIONPROC)(u32 program, s8 *name);
typedef void (*PFNGLUNIFORM1FPROC)(i32 location, f32 v0);
typedef void (*PFNGLUNIFORM1IPROC)(i32 location, i32 v0);
typedef void (*PFNGLUNIFORM2FPROC)(i32 location, f32 v0, f32 v1);
typedef void (*PFNGLUNIFORM3FPROC)(i32 location, f32 v0, f32 v1, f32 v2);
typedef void (*PFNGLUNIFORM3IPROC)(i32 location, i32 v0, i32 v1, i32 v2);
typedef void (*PFNGLUNIFORM4FPROC)(i32 location, f32 v0, f32 v1, f32 v2, f32 v3);
typedef void (*PFNGLUNIFORM4FVPROC)(i32 location, i32 count, f32 *value);
typedef void (*PFNGLUNIFORMMATRIX3FVPROC)(i32 location, i32 count, u8 transpose, f32 *value);
typedef void (*PFNGLUNIFORMMATRIX4FVPROC)(i32 location, i32 count, u8 transpose, f32 *value);
typedef void (*PFNGLACTIVETEXTUREPROC)(u32 texture);
typedef void (*PFNGLTEXIMAGE3DPROC)(u32 target, i32 level, i32 internalFormat, i32 width, i32 height, i32 depth, i32 border, u32 format, u32 type, void *data);
typedef void (*PFNGLGENERATEMIPMAPPROC)(u32 target);
typedef void (*PFNGLGENBUFFERSPROC)(i32 n, u32 *buffers);
typedef void (*PFNGLBINDBUFFERPROC)(u32 target, u32 buffer);
typedef void (*PFNGLBUFFERDATAPROC)(u32 target, i32 size, void *data, u32 usage);
typedef void (*PFNGLBUFFERSUBDATAPROC)(u32 target, i32 *offset, i32 size, void *data);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(u32 index);
typedef void (*PFNGLVERTEXATTRIBPOINTERPROC)(u32 index, i32 size, u32 type, u8 normalized, i32 stride, void *pointer);
typedef void (*PFNGLVERTEXATTRIBIPOINTERPROC)(u32 index, i32 size, u32 type, i32 stride, void *pointer);
typedef void (*PFNGLVERTEXATTRIBDIVISORPROC)(u32 index, u32 divisor);
typedef void (*PFNGLDRAWARRAYSINSTANCED)(i32 mode, i32 first, i32 count, u32 primcount);
typedef void (*PFNGLGENFRAMEBUFFERSPROC)(i32 n, u32 *ids);
typedef void (*PFNGLDELETEFRAMEBUFFERSPROC)(i32 n, u32 *framebuffers);
typedef void (*PFNGLBINDFRAMEBUFFERPROC)(u32 target, u32 framebuffer);
typedef void (*PFNGLFRAMEBUFFERTEXTURE2DPROC)(u32 target, u32 attachment, u32 textarget, u32 texture, i32 level);
typedef void (*PFNGLFRAMEBUFFERTEXTURELAYERPROC)(u32 target, u32 attachment, u32 texture, i32 level, i32 layer);

typedef struct vast_opengl_api
{
    /* OpenGL 1.1 functions */
    PFNGLCLEARCOLORPROC glClearColor;
    PFNGLCLEARPROC glClear;
    PFNGLPOLYGONMODEPROC glPolygonMode;
    PFNGLVIEWPORTPROC glViewport;
    PFNGLENABLEPROC glEnable;
    PFNGLDISABLEPROC glDisable;
    PFNGLGETSTRINGPROC glGetString;
    PFNGLGENTEXTURESPROC glGenTextures;
    PFNGLBINDTEXTUREPROC glBindTexture;
    PFNGLTEXIMAGE1DPROC glTexImage1D;
    PFNGLTEXIMAGE2DPROC glTexImage2D;
    PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
    PFNGLTEXPARAMETERIPROC glTexParameteri;
    PFNGLPIXELSTOREIPROC glPixelStorei;
    PFNGLREADPIXELSPROC glReadPixels;
    PFNGLBLENDFUNCPROC glBlendFunc;
    PFNGLGETINTEGERVPROC glGetIntegerv;
    PFNGLCULLFACEPROC glCullFace;
    PFNGLDRAWELEMENTSPROC glDrawElements;
    PFNGLDEPTHMASKPROC glDepthMask;
    PFNGLDEPTHFUNCPROC glDepthFunc;

    /* Opengl 1.1+ until current */
    PFNGLCREATESHADERPROC glCreateShader;
    PFNGLCREATEPROGRAMPROC glCreateProgram;
    PFNGLDELETEPROGRAMPROC glDeleteProgram;
    PFNGLATTACHSHADERPROC glAttachShader;
    PFNGLSHADERSOURCEPROC glShaderSource;
    PFNGLCOMPILESHADERPROC glCompileShader;
    PFNGLGETSHADERIVPROC glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    PFNGLLINKPROGRAMPROC glLinkProgram;
    PFNGLGETPROGRAMIVPROC glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    PFNGLDELETESHADERPROC glDeleteShader;
    PFNGLDRAWARRAYSPROC glDrawArrays;
    PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
    PFNGLUSEPROGRAMPROC glUseProgram;
    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    PFNGLUNIFORM1FPROC glUniform1f;
    PFNGLUNIFORM1IPROC glUniform1i;
    PFNGLUNIFORM2FPROC glUniform2f;
    PFNGLUNIFORM3FPROC glUniform3f;
    PFNGLUNIFORM3IPROC glUniform3i;
    PFNGLUNIFORM4FPROC glUniform4f;
    PFNGLUNIFORM4FVPROC glUniform4fv;
    PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
    PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
    PFNGLACTIVETEXTUREPROC glActiveTexture;
    PFNGLTEXIMAGE3DPROC glTexImage3D;
    PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLBUFFERDATAPROC glBufferData;
    PFNGLBUFFERSUBDATAPROC glBufferSubData;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
    PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
    PFNGLDRAWARRAYSINSTANCED glDrawArraysInstanced;
    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
    PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
    PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;

} vast_opengl_api;

#endif /* VAST_OPENGL_API_H */