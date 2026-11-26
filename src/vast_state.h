#ifndef VAST_STATE_H
#define VAST_STATE_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...) */
#include "vast.h"                     /* Platform memory struct     */
#include "vast_math_linear_algebra.h" /* Vectors/Matrices           */
#include "vast_camera.h"              /* Camera data                */

/* #############################################################################
 * # [SECTION] State Single source of truth
 * #############################################################################
 */
typedef struct vast_state
{
    /* Memory Regions */
    vast_platform_memory memory_region_permanent;
    vast_platform_memory memory_region_transient;

    /* VRam Usage */
    u32 memory_vram_usage;

    /* Rendering Toggle Flags */
    u8 rendering_enabled_terrain;
    u8 rendering_enabled_water;
    u8 rendering_enabled_sky;
    u8 rendering_enabled_post;
    u8 rendering_enabled_ui;

    /* Rendering Draw Calls */
    u32 rendering_draw_calls_terrain;
    u32 rendering_draw_calls_water;
    u32 rendering_draw_calls_sky;
    u32 rendering_draw_calls_post;
    u32 rendering_draw_calls_ui;

    /* Camera */
    vast_camera camera;
    f32 camera_speed;

    /* Mouse */
    f32 mouse_sensitivity;

    /* View and Frustum data */
    vast_mat4x4 projection;
    vast_mat4x4 view;
    vast_mat4x4 view_projection;
    vast_frustum view_frustum;

    /* Ligthing and Weather data */
    vast_vec3 sun_direction;
    vast_vec3 wind_direction;

    /* Post processing fields */
    u32 post_framebuffer_object_id;

    /* UI information */
    f32 ui_scale;   /* Default 1.0f */
    f32 ui_padding; /* Default 8.0f */
    u32 ui_font_texture_width;
    u32 ui_font_texture_height;
    f32 ui_font_size;
    f32 ui_font_advancement_factor;

} vast_state;

#endif /* VAST_STATE_H */
