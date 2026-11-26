#ifndef VAST_UI_RENDERER_H
#define VAST_UI_RENDERER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)    */
#include "vast.h"                     /* Platform State                */
#include "vast_state.h"               /* Vast state struct             */
#include "vast_ui.h"                  /* Immediate Mode UI API         */
#include "vast_profiler.h"            /* Profiler metrics              */
#include "vast_math_linear_algebra.h" /* Linear Algebra                */
#include "vast_camera.h"              /* Camera                        */
#include "vast_string_builder.h"      /* String parsing                */
#include "vast_opengl.h"              /* OpenGL functions              */

/* #############################################################################
 * # [SECTION] Structs and Data
 * #############################################################################
 */
typedef struct vast_ui_shader
{
    shader_header header;

    i32 loc_iResolution;
    i32 loc_iTime;
    i32 loc_texture_font;

} vast_ui_shader;

/* #############################################################################
 * # [SECTION] UI Renderer
 * #############################################################################
 */
VAST_API VAST_INLINE u32 vast_ui_profiler_color(f64 ms)
{
    if (ms < 0.1)
    {
        return 0xFFFFFFFF; /* Normal */
    }
    else if (ms < 1.0)
    {
        return 0xFF00AAFF; /* Orange */
    }
    else
    {
        return 0xFF3333FF; /* Red */
    }
}

VAST_API VAST_INLINE u32 vast_ui_color_darken(u32 color, f32 factor)
{
    u32 a = color & 0xFF000000u;
    u32 b = (u32)((f32)((color >> 16) & 0xFFu) * factor);
    u32 g = (u32)((f32)((color >> 8) & 0xFFu) * factor);
    u32 r = (u32)((f32)(color & 0xFFu) * factor);

    if (b > 0xFFu)
    {
        b = 0xFFu;
    }

    if (g > 0xFFu)
    {
        g = 0xFFu;
    }

    if (r > 0xFFu)
    {
        r = 0xFFu;
    }

    return a | ((b & 0xFFu) << 16) | ((g & 0xFFu) << 8) | (r & 0xFFu);
}

VAST_API void vast_ui_window_tile(vast_platform_state *platform_state, vast_state *state, vast_ui_context *ui_ctx, f32 font_size)
{
    static s8 buffer[128];
    vast_sb sb = {sizeof(buffer), 0, buffer};

    vast_vec4 tile_bounds = vast_tile_calculate_bounds(&state->tiles, state->camera.position.x, state->camera.position.z);

    f32 win_w = 220.0f;
    f32 win_h = 320.0f;
    f32 win_x = (f32)platform_state->window.width - win_w;
    f32 win_y = 180.0f;

    vast_ui_result header;
    vast_ui_result panel;

    header = vast_ui_drag_header(ui_ctx, 40, &win_x, &win_y, win_w, win_h, font_size);
    panel = vast_ui_panel_begin(ui_ctx, vast_ui_rect_init(win_x, win_y, win_w, win_h));

    vast_ui_push_rect(ui_ctx, panel, 0xEE1E1E26);
    vast_ui_push_rect(ui_ctx, header, 0xFF333344);
    vast_ui_push_text(ui_ctx, "Tile Info", header.rect, 0xFFFFFFFF);

    (void)state;

    /* Space past header */
    ui_ctx->cursor_y += font_size;

    vast_sb_s8_pad(&sb, "Name", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_s8_pad(&sb, "Value", 10, ' ', VAST_SB_PAD_LEFT);
    vast_sb_s8(&sb, "\0");
    vast_ui_label(ui_ctx, buffer, 0xFFAAAAAA);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "origin_x", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_i32_pad(&sb, state->tiles.origin_x, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "origin_z", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_i32_pad(&sb, state->tiles.origin_z, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "camera_x", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_i32_pad(&sb, state->tiles.camera_x, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "camera_z", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_i32_pad(&sb, state->tiles.camera_z, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "tile_size", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_f64_pad(&sb, (f64)state->tiles.world_size, 4, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "lod_max", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_i32_pad(&sb, state->tiles.tile_lod_max, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "dirty_count", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_i32_pad(&sb, state->tiles.dirty_indices_count, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "x_min", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_f64_pad(&sb, (f64)tile_bounds.x, 4, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "x_max", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_f64_pad(&sb, (f64)tile_bounds.y, 4, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "z_min", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_f64_pad(&sb, (f64)tile_bounds.z, 4, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_sb_s8_pad(&sb, "z_max", 12, ' ', VAST_SB_PAD_RIGHT);
    vast_sb_f64_pad(&sb, (f64)tile_bounds.w, 4, 10, ' ', VAST_SB_PAD_LEFT);
    vast_ui_label(ui_ctx, buffer, 0xFFFFFFFF);
    sb.length = 0;

    vast_ui_panel_end(ui_ctx);
}

VAST_API void vast_ui_render(vast_platform_state *platform_state, vast_state *state)
{
    static u8 ui_initialized = 0;

    static vast_ui_context ui_ctx = {0};
    static vast_ui_shader ui_shader = {0};
    static u32 ui_vao = 0;
    static u32 ui_vbo = 0;
    static u32 font_texture = 0;
    static u32 font_texture_width = 1024;
    static u32 font_texture_height = 1024;

    vast_opengl_api *gl = &platform_state->graphics.opengl;

    if (!ui_initialized)
    {
        u8 *shader_data_vertex = (u8 *)vast_io_file_read_to_memory(&platform_state->api, &state->memory_region_transient, "shaders/50_vast_ui.vs");
        u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&platform_state->api, &state->memory_region_transient, "shaders/50_vast_ui.fs");
        u8 *font_data = (u8 *)vast_io_file_read_to_memory(&platform_state->api, &state->memory_region_transient, "assets/font.raw");

#define INSTANCE_DATA_COUNT 1024
        static vast_ui_instance_data instance_data[INSTANCE_DATA_COUNT];

        if (!shader_data_vertex || !shader_data_fragment || !font_data)
        {
            return;
        }

        /* Setup ui context */
        ui_ctx.instances = instance_data;
        ui_ctx.instances_count = 0;
        ui_ctx.instances_size = INSTANCE_DATA_COUNT;

        /* Load Shader */
        if (opengl_shader_load(&ui_shader.header, (s8 *)shader_data_vertex, (s8 *)shader_data_fragment, platform_state->api.io_print, gl))
        {
            ui_shader.loc_iResolution = gl->glGetUniformLocation(ui_shader.header.program, "iResolution");
            ui_shader.loc_iTime = gl->glGetUniformLocation(ui_shader.header.program, "iTime");
            ui_shader.loc_texture_font = gl->glGetUniformLocation(ui_shader.header.program, "tex_font");
        }
        else
        {
            platform_state->api.io_print("Cannot compile font shaders!\n");
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

        /* Setup Instanced VAO + VBO */
        gl->glGenVertexArrays(1, &ui_vao);
        gl->glBindVertexArray(ui_vao);

        gl->glGenBuffers(1, &ui_vbo);
        gl->glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(instance_data), 0, GL_DYNAMIC_DRAW);

        /* location 0: a_rect (pos_x, pos_y, size_x, size_y) */
        gl->glEnableVertexAttribArray(0);
        gl->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vast_ui_instance_data), (void *)0);
        gl->glVertexAttribDivisor(0, 1);

        /* location 1: a_color */
        gl->glEnableVertexAttribArray(1);
        gl->glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vast_ui_instance_data), (void *)(4 * sizeof(f32)));
        gl->glVertexAttribDivisor(1, 1);

        /* location 2: a_glyph_index */
        gl->glEnableVertexAttribArray(2);
        gl->glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(vast_ui_instance_data), (void *)(4 * sizeof(f32) + sizeof(u32)));
        gl->glVertexAttribDivisor(2, 1);

        gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        gl->glBindVertexArray(0);

        ui_initialized = 1;
    }

    /* Update */
    {
        static f32 win_x = 0.0f;
        static f32 win_y = 0.0f;

        f32 win_w = 180.0f;
        f32 win_h = 240.0f;

        static f32 font_size = 16.0f;
        static f32 font_advancement_factor = 0.55f;
        static f32 ui_scale = 1.0f;
        static f32 ui_padding = 8.0f;

        vast_ui_result header;
        vast_ui_result panel;

        VAST_PROFILER_BEGIN(font_update);

        vast_ui_begin(&ui_ctx,
                      platform_state->window.width, platform_state->window.height,
                      (f32)platform_state->input.mouse.x,
                      (f32)platform_state->window.height - (f32)platform_state->input.mouse.y,
                      platform_state->input.mouse.keys_is_down[VAST_MOUSE_KEY_LEFT],
                      ui_scale, ui_padding,
                      font_size, font_advancement_factor);

        header = vast_ui_drag_header(&ui_ctx, 1, &win_x, &win_y, win_w, win_h, font_size);
        panel = vast_ui_panel_begin(&ui_ctx, vast_ui_rect_init(win_x, win_y, win_w, win_h));
        vast_ui_push_rect(&ui_ctx, panel, 0xEE1E1E26);

        {
            vast_ui_push_rect(&ui_ctx, header, 0xFF333344);
            vast_ui_push_text(&ui_ctx, "Vast UI (F1)", header.rect, 0xFFFFFFFF);

            /* Space past header */
            ui_ctx.cursor_y += font_size;

            /* Post Process Button */
            {
                vast_ui_result btn = vast_ui_button(&ui_ctx, 2, vast_ui_rect_init(0.0f, 0.0f, 17 * font_size * 0.55f, font_size));

                u32 btn_color = state->rendering_enabled_post ? 0xFF00AA00 : 0xFF0000AA;

                if (btn.state & VAST_UI_STATE_HOVER)
                {
                    btn_color = vast_ui_color_darken(btn_color, 0.8f);
                }

                if (btn.state & VAST_UI_STATE_PRESSED)
                {
                    state->rendering_enabled_post = !state->rendering_enabled_post;
                }

                vast_ui_push_rect(&ui_ctx, btn, btn_color);
                vast_ui_push_text(&ui_ctx, "Post Process (B)", btn.rect, 0xFFFFFFFF);
            }

            /* Scene Render Terrain */
            {
                vast_ui_result btn = vast_ui_button(&ui_ctx, 2, vast_ui_rect_init(0.0f, 0.0f, 17 * font_size * 0.55f, font_size));

                u32 btn_color = state->rendering_enabled_terrain ? 0xFF00AA00 : 0xFF0000AA;

                if (btn.state & VAST_UI_STATE_HOVER)
                {
                    btn_color = vast_ui_color_darken(btn_color, 0.8f);
                }

                if (btn.state & VAST_UI_STATE_PRESSED)
                {
                    state->rendering_enabled_terrain = !state->rendering_enabled_terrain;
                }

                vast_ui_push_rect(&ui_ctx, btn, btn_color);
                vast_ui_push_text(&ui_ctx, "Render Terrain", btn.rect, 0xFFFFFFFF);
            }

            /* Scene Render Water */
            {
                vast_ui_result btn = vast_ui_button(&ui_ctx, 2, vast_ui_rect_init(0.0f, 0.0f, 17 * font_size * 0.55f, font_size));

                u32 btn_color = state->rendering_enabled_water ? 0xFF00AA00 : 0xFF0000AA;

                if (btn.state & VAST_UI_STATE_HOVER)
                {
                    btn_color = vast_ui_color_darken(btn_color, 0.8f);
                }

                if (btn.state & VAST_UI_STATE_PRESSED)
                {
                    state->rendering_enabled_water = !state->rendering_enabled_water;
                }

                vast_ui_push_rect(&ui_ctx, btn, btn_color);
                vast_ui_push_text(&ui_ctx, "Render Water", btn.rect, 0xFFFFFFFF);
            }

            /* Scene Render Sky */
            {
                vast_ui_result btn = vast_ui_button(&ui_ctx, 2, vast_ui_rect_init(0.0f, 0.0f, 17 * font_size * 0.55f, font_size));

                u32 btn_color = state->rendering_enabled_sky ? 0xFF00AA00 : 0xFF0000AA;

                if (btn.state & VAST_UI_STATE_HOVER)
                {
                    btn_color = vast_ui_color_darken(btn_color, 0.8f);
                }

                if (btn.state & VAST_UI_STATE_PRESSED)
                {
                    state->rendering_enabled_sky = !state->rendering_enabled_sky;
                }

                vast_ui_push_rect(&ui_ctx, btn, btn_color);
                vast_ui_push_text(&ui_ctx, "Render Sky", btn.rect, 0xFFFFFFFF);
            }
        }

        /* FPS Setting */
        {
            vast_ui_result btn_minus = vast_ui_button(&ui_ctx, 3, vast_ui_rect_init(0.0f, 0.0f, 2 * font_size * 0.55f, font_size));
            vast_ui_result btn_plus = vast_ui_button(&ui_ctx, 4, vast_ui_rect_init(0.0f, 0.0f, 2 * font_size * 0.55f, font_size));

            u32 btn_minus_color = btn_minus.state & VAST_UI_STATE_HOVER ? 0xFF333344 : 0xFF181818;
            u32 btn_plus_color = btn_plus.state & VAST_UI_STATE_HOVER ? 0xFF333344 : 0xFF181818;

            if (btn_minus.state & VAST_UI_STATE_PRESSED)
            {
                platform_state->timing.frame_rate_target = (u32)vast_maxf((f32)platform_state->timing.frame_rate_target - 10.0f, 0.0f);

                /*
                ui_scale -= 0.1f;
                ui_scale = vast_maxf(0.5f, ui_scale);
                */
            }

            if (btn_plus.state & VAST_UI_STATE_PRESSED)
            {
                platform_state->timing.frame_rate_target = (u32)vast_minf((f32)platform_state->timing.frame_rate_target + 10.0f, 2000.0f);

                /*
                ui_scale += 0.1f;
                ui_scale = vast_minf(2.0f, ui_scale);
                */
            }

            vast_ui_push_rect(&ui_ctx, btn_minus, btn_minus_color);
            vast_ui_push_text(&ui_ctx, "-", btn_minus.rect, 0xFFFFFFFF);

            vast_ui_push_rect(&ui_ctx, btn_plus, btn_plus_color);
            vast_ui_push_text(&ui_ctx, "+", btn_plus.rect, 0xFFFFFFFF);
        }

        /* FPS */
        {
            static s8 buffer[64];

            vast_sb sb = {0};
            sb.buffer = buffer;
            sb.size = 64;

            vast_sb_s8(&sb, "FPS   : ");
            vast_sb_i32_pad(&sb, (i32)platform_state->timing.frame_rate, 8, ' ', VAST_SB_PAD_LEFT);
            vast_sb_s8(&sb, "\0");

            vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

            sb.length = 0;

            vast_sb_s8(&sb, "FPSCAP: ");
            vast_sb_i32_pad(&sb, (i32)platform_state->timing.frame_rate_target, 8, ' ', VAST_SB_PAD_LEFT);
            vast_sb_s8(&sb, "\0");

            vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

            sb.length = 0;

            vast_sb_s8(&sb, "DELTA : ");
            vast_sb_f64_pad(&sb, (f64)platform_state->timing.time_delta, 4, 8, ' ', VAST_SB_PAD_LEFT);
            vast_sb_s8(&sb, "\0");

            vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

            sb.length = 0;
        }

        vast_ui_panel_end(&ui_ctx);
        VAST_PROFILER_END(font_update);

        /* Profiler Metrics Window */
        {
            static f32 prof_win_w = 620.0f;
            static f32 prof_win_h = 310.0f;
            static f32 prof_win_x = 0.0f;

            f32 prof_win_y = (f32)platform_state->window.height - (prof_win_h + prof_win_x);

            vast_ui_result prof_header;
            vast_ui_result prof_panel;

            prof_header = vast_ui_drag_header(&ui_ctx, 10, &prof_win_x, &prof_win_y, prof_win_w, prof_win_h, font_size);
            prof_panel = vast_ui_panel_begin(&ui_ctx, vast_ui_rect_init(prof_win_x, prof_win_y, prof_win_w, prof_win_h));
            vast_ui_push_rect(&ui_ctx, prof_panel, 0xEE1E1E26);

            {
                vast_ui_push_rect(&ui_ctx, prof_header, 0xFF333344);
                vast_ui_push_text(&ui_ctx, "Profiler Metrics (milliseconds)", prof_header.rect, 0xFFFFFFFF);

                /* Space past header */
                ui_ctx.cursor_y += font_size;

                if (vast_profiler_entries_count > 0)
                {
                    u32 i;

                    for (i = 0; i < vast_profiler_entries_count; ++i)
                    {
                        vast_profiler_entry entry = vast_profiler_entries[i];
                        static s8 prof_buf[256];
                        vast_sb sb = {0};
                        sb.buffer = prof_buf;
                        sb.size = sizeof(prof_buf);
                        sb.length = 0;

                        if (i == 0)
                        {
                            vast_sb_s8_pad(&sb, "Name", 24, ' ', VAST_SB_PAD_RIGHT);
                            vast_sb_s8_pad(&sb, "last", 9, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8_pad(&sb, "min", 9, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8_pad(&sb, "max", 9, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8_pad(&sb, "avg", 9, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8_pad(&sb, "cnt", 7, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8(&sb, "\0");

                            vast_ui_label(&ui_ctx, prof_buf, 0xFFAAAAAA);
                            sb.length = 0;
                        }

                        vast_sb_s8_pad(&sb, entry.name, 24, ' ', VAST_SB_PAD_RIGHT);
                        vast_sb_f64_pad(&sb, entry.time_ms_last, 4, 9, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_f64_pad(&sb, entry.time_ms_min, 4, 9, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_f64_pad(&sb, entry.time_ms_max, 4, 9, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_f64_pad(&sb, entry.counter > 0 ? (entry.time_ms_total / (f64)entry.counter) : 0.0, 4, 9, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_i32_pad(&sb, (i32)entry.counter, 7, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_s8(&sb, "\0");

                        vast_ui_label(&ui_ctx, prof_buf, vast_ui_profiler_color(entry.time_ms_last));
                    }
                }
                else
                {
                    vast_ui_label(&ui_ctx, "No profiler entries found.", 0xFF888888);
                }
            }
            vast_ui_panel_end(&ui_ctx);
        }

        /* Camera Information Window */
        {
            static f32 cam_win_w = 320.0f;
            static f32 cam_win_h = 220.0f;
            static f32 cam_win_x = 180.0f + 10.0f;
            static f32 cam_win_y = 0.0f;

            vast_ui_result cam_header;
            vast_ui_result cam_panel;

            cam_header = vast_ui_drag_header(&ui_ctx, 20, &cam_win_x, &cam_win_y, cam_win_w, cam_win_h, font_size);
            cam_panel = vast_ui_panel_begin(&ui_ctx, vast_ui_rect_init(cam_win_x, cam_win_y, cam_win_w, cam_win_h));
            vast_ui_push_rect(&ui_ctx, cam_panel, 0xEE1E1E26);

            {
                vast_ui_push_rect(&ui_ctx, cam_header, 0xFF333344);
                vast_ui_push_text(&ui_ctx, "Camera Information", cam_header.rect, 0xFFFFFFFF);

                /* Space past header */
                ui_ctx.cursor_y += font_size;

                {
                    static s8 buffer[64];

                    vast_sb sb = {0};
                    sb.buffer = buffer;
                    sb.size = sizeof(buffer);

                    vast_sb_s8(&sb, "Position: ");
                    vast_sb_f64_pad(&sb, (f64)state->camera.position.x, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "/");
                    vast_sb_f64_pad(&sb, (f64)state->camera.position.y, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "/");
                    vast_sb_f64_pad(&sb, (f64)state->camera.position.z, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8(&sb, "Forward : ");
                    vast_sb_f64_pad(&sb, (f64)state->camera.forward.x, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "/");
                    vast_sb_f64_pad(&sb, (f64)state->camera.forward.y, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "/");
                    vast_sb_f64_pad(&sb, (f64)state->camera.forward.z, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8(&sb, "Up      : ");
                    vast_sb_f64_pad(&sb, (f64)state->camera.up.x, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "/");
                    vast_sb_f64_pad(&sb, (f64)state->camera.up.y, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "/");
                    vast_sb_f64_pad(&sb, (f64)state->camera.up.z, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8(&sb, "Right   : ");
                    vast_sb_f64_pad(&sb, (f64)state->camera.right.x, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "/");
                    vast_sb_f64_pad(&sb, (f64)state->camera.right.y, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "/");
                    vast_sb_f64_pad(&sb, (f64)state->camera.right.z, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8(&sb, "Yaw     : ");
                    vast_sb_f64_pad(&sb, (f64)state->camera.yaw, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8(&sb, "Pitch   : ");
                    vast_sb_f64_pad(&sb, (f64)state->camera.pitch, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8(&sb, "Roll    : ");
                    vast_sb_f64_pad(&sb, (f64)state->camera.roll, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8(&sb, "Fov     : ");
                    vast_sb_f64_pad(&sb, (f64)state->camera.fov, 2, 7, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;
                }
            }
            vast_ui_panel_end(&ui_ctx);
        }

        /* Memory Information Window */
        {
            static f32 mem_win_w = 280.0f;
            static f32 mem_win_h = 160.0f;

            f32 mem_win_x = (f32)platform_state->window.width - mem_win_w;
            f32 mem_win_y = 0.0f;

            vast_ui_result mem_header;
            vast_ui_result mem_panel;

            mem_header = vast_ui_drag_header(&ui_ctx, 30, &mem_win_x, &mem_win_y, mem_win_w, mem_win_h, font_size);
            mem_panel = vast_ui_panel_begin(&ui_ctx, vast_ui_rect_init(mem_win_x, mem_win_y, mem_win_w, mem_win_h));
            vast_ui_push_rect(&ui_ctx, mem_panel, 0xEE1E1E26);

            {
                vast_ui_push_rect(&ui_ctx, mem_header, 0xFF333344);
                vast_ui_push_text(&ui_ctx, "Memory Information (bytes)", mem_header.rect, 0xFFFFFFFF);

                /* Space past header */
                ui_ctx.cursor_y += font_size;

                {
                    static s8 buffer[128];

                    vast_sb sb = {0};
                    sb.buffer = buffer;
                    sb.size = sizeof(buffer);

                    vast_sb_s8_pad(&sb, "Name", 11, ' ', VAST_SB_PAD_RIGHT);
                    vast_sb_s8_pad(&sb, "total", 8, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8_pad(&sb, "used", 10, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, "\0");

                    vast_ui_label(&ui_ctx, buffer, 0xFFAAAAAA);

                    sb.length = 0;

                    vast_sb_s8_pad(&sb, "cpu_perm", 11, ' ', VAST_SB_PAD_RIGHT);
                    vast_sb_i32_pad(&sb, (i32)state->memory_region_permanent.size, 8, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_i32_pad(&sb, (i32)state->memory_region_permanent.offset, 10, ' ', VAST_SB_PAD_LEFT);

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8_pad(&sb, "cpu_trans", 11, ' ', VAST_SB_PAD_RIGHT);
                    vast_sb_i32_pad(&sb, (i32)state->memory_region_transient.size, 8, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_i32_pad(&sb, (i32)state->memory_region_transient.offset, 10, ' ', VAST_SB_PAD_LEFT);

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;
                }

                /* Space past mem table */
                ui_ctx.cursor_y += font_size;

                {
                    static s8 buffer[32];

                    vast_sb sb = {0};
                    sb.buffer = buffer;
                    sb.size = sizeof(buffer);

                    vast_sb_s8_pad(&sb, "cpu_ram", 11, ' ', VAST_SB_PAD_RIGHT);
                    vast_sb_f64_pad(&sb, (f64)(state->memory_region_permanent.offset + state->memory_region_transient.offset) / 1024.0 / 1024.0, 4, 8, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, " MB");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;

                    vast_sb_s8_pad(&sb, "gpu_vram", 11, ' ', VAST_SB_PAD_RIGHT);
                    vast_sb_f64_pad(&sb, (f64)state->memory_vram_usage / 1024.0 / 1024.0, 4, 8, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&sb, " MB");

                    vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

                    sb.length = 0;
                }
            }
            vast_ui_panel_end(&ui_ctx);
        }

        vast_ui_window_tile(platform_state, state, &ui_ctx, font_size);

        vast_ui_end(&ui_ctx);
    }

    if (ui_ctx.instances_count == 0)
    {
        return;
    }

    /* Render */
    VAST_PROFILER_BEGIN(font_render);
    {
        gl->glDisable(GL_DEPTH_TEST);
        gl->glEnable(GL_BLEND);
        gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        gl->glUseProgram(ui_shader.header.program);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, font_texture);

        gl->glUniform3f(ui_shader.loc_iResolution, (f32)platform_state->window.width, (f32)platform_state->window.height, 1.0f);
        gl->glUniform1f(ui_shader.loc_iTime, (f32)platform_state->timing.time_elapsed);
        gl->glUniform1i(ui_shader.loc_texture_font, 0);

        gl->glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
        gl->glBufferSubData(GL_ARRAY_BUFFER, 0, (i32)ui_ctx.instances_count * (i32)sizeof(vast_ui_instance_data), ui_ctx.instances);
        gl->glBindBuffer(GL_ARRAY_BUFFER, 0);

        gl->glBindVertexArray(ui_vao);
        gl->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ui_ctx.instances_count);
        gl->glBindVertexArray(0);

        gl->glDisable(GL_BLEND);
        gl->glEnable(GL_DEPTH_TEST);
    }
    VAST_PROFILER_END(font_render);
}

#endif /* VAST_UI_RENDERER_H */