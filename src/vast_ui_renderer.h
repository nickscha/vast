#ifndef VAST_UI_RENDERER_H
#define VAST_UI_RENDERER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"               /* Basic types (u8, u32, ...)    */
#include "vast.h"                     /* Platform State                */
#include "vast_profiler.h"            /* Profiler metrics              */
#include "vast_math_linear_algebra.h" /* Linear Algebra                */
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

typedef struct vast_ui_instance_data
{
    f32 pos_x;       /* Screen X in pixels (or NDC) */
    f32 pos_y;       /* Screen Y in pixels */
    f32 size_x;      /* Glyph width in pixels */
    f32 size_y;      /* Glyph height in pixels */
    u32 color;       /* Packed RGBA8 (0xAABBGGRR) */
    u32 glyph_index; /* 0..255 index into the 16x16 atlas */

} vast_ui_instance_data;

/* #############################################################################
 * # [SECTION] Immediate Mode API
 * #############################################################################
 */
#ifndef VAST_UI_MAX_STACK
#define VAST_UI_MAX_STACK 8
#endif

#define VAST_UI_SOLID_FILL_MAGIC_INDEX 424242u

typedef struct vast_ui_rect
{
    f32 x;
    f32 y;
    f32 w;
    f32 h;

} vast_ui_rect;

typedef struct vast_ui_context
{
    u32 window_width;
    u32 window_height;

    f32 mouse_x;
    f32 mouse_y;
    f32 mouse_x_prev;
    f32 mouse_y_prev;

    f32 cursor_y;
    f32 padding;
    f32 scale;

    f32 font_size;
    f32 font_advance_factor;

    u32 active_id;
    u32 dragged_window_id;
    f32 drag_offset_x;
    f32 drag_offset_y;

    u8 mouse_down;
    u8 mouse_pressed;
    u8 mouse_released;
    u8 mouse_was_down;

    i32 stack_ptr;
    vast_ui_rect stack[VAST_UI_MAX_STACK];

    /* Passed instances buffer */
    vast_ui_instance_data *instances;
    u32 instances_count;
    u32 instances_size;

} vast_ui_context;

typedef enum vast_ui_state
{
    VAST_UI_STATE_IDLE = 0,
    VAST_UI_STATE_HOVER = (1 << 0),
    VAST_UI_STATE_PRESSED = (1 << 1),
    VAST_UI_STATE_HELD = (1 << 2),
    VAST_UI_STATE_RELEASED = (1 << 3)
} vast_ui_state;

typedef struct vast_ui_result
{
    vast_ui_rect rect;
    u8 state;
} vast_ui_result;

VAST_API VAST_INLINE vast_ui_rect vast_ui_rect_init(f32 x, f32 y, f32 width, f32 height)
{
    vast_ui_rect result;

    result.x = x;
    result.y = y;
    result.w = width;
    result.h = height;

    return result;
}

VAST_API VAST_INLINE void vast_ui_begin(
    vast_ui_context *ctx,
    u32 window_width, u32 window_height,
    f32 mouse_x, f32 mouse_y, u8 mouse_down,
    f32 scale, f32 padding,
    f32 font_size, f32 font_advance_factor)
{
    ctx->window_width = window_width;
    ctx->window_height = window_height;

    ctx->mouse_x_prev = ctx->mouse_x;
    ctx->mouse_y_prev = ctx->mouse_y;
    ctx->mouse_x = mouse_x;
    ctx->mouse_y = mouse_y;

    ctx->mouse_was_down = ctx->mouse_down;
    ctx->mouse_down = mouse_down;
    ctx->mouse_pressed = ctx->mouse_down && !ctx->mouse_was_down;
    ctx->mouse_released = !ctx->mouse_down && ctx->mouse_was_down;

    ctx->scale = scale > 0.0f ? scale : 1.0f;
    ctx->padding = padding > 0.0f ? padding : 8.0f;

    ctx->font_size = font_size > 0.0f ? font_size : 16.0f;
    ctx->font_advance_factor = font_advance_factor > 0.0f ? font_advance_factor : 0.55f;

    ctx->stack_ptr = 0;
    ctx->cursor_y = 0.0f;
    ctx->instances_count = 0;

    if (!ctx->mouse_down)
    {
        ctx->active_id = 0;
        ctx->dragged_window_id = 0;
    }
}

VAST_API VAST_INLINE void vast_ui_end(vast_ui_context *ctx)
{
    (void)ctx;
}

VAST_API VAST_INLINE vast_ui_result vast_ui_internal_process(vast_ui_context *ctx, u32 id, vast_ui_rect rect)
{
    vast_ui_result res;
    f32 s = ctx->scale;
    f32 pad = ctx->padding * s;
    u8 is_over;

    res.state = VAST_UI_STATE_IDLE;

    /* Auto-layout if x and y are 0 and a panel container is open */
    if (rect.x == 0.0f && rect.y == 0.0f && ctx->stack_ptr > 0)
    {
        vast_ui_rect *panel = &ctx->stack[ctx->stack_ptr - 1];
        f32 pad2 = pad * 2.0f;

        res.rect.x = panel->x + pad;
        res.rect.y = panel->y + ctx->cursor_y + pad;
        res.rect.w = (rect.w > 0.0f) ? (rect.w * s) : (panel->w - pad2);
        res.rect.h = rect.h * s;

        ctx->cursor_y += res.rect.h + pad;
    }
    else
    {
        res.rect.x = rect.x * s;
        res.rect.y = rect.y * s;
        res.rect.w = rect.w * s;
        res.rect.h = rect.h * s;
    }

    /* Clip against active panel bounds */
    if (ctx->stack_ptr > 0)
    {
        vast_ui_rect *panel = &ctx->stack[ctx->stack_ptr - 1];
        if (res.rect.y + res.rect.h < panel->y || res.rect.y > panel->y + panel->h ||
            res.rect.x + res.rect.w < panel->x || res.rect.x > panel->x + panel->w)
        {
            return res; /* Outside clip region */
        }
    }

    if (ctx->active_id && ctx->active_id != id)
    {
        return res;
    }

    is_over = (ctx->mouse_x >= res.rect.x && ctx->mouse_x <= (res.rect.x + res.rect.w) &&
               ctx->mouse_y >= res.rect.y && ctx->mouse_y <= (res.rect.y + res.rect.h));

    if (is_over)
    {
        res.state |= VAST_UI_STATE_HOVER;
        if (!ctx->active_id && ctx->mouse_pressed)
        {
            ctx->active_id = id;
            res.state |= VAST_UI_STATE_PRESSED;
        }
    }

    if (ctx->active_id == id)
    {
        res.state |= VAST_UI_STATE_HELD;
        if (ctx->mouse_released)
        {
            ctx->active_id = 0;
            if (is_over)
            {
                res.state |= VAST_UI_STATE_RELEASED;
            }
        }
    }

    return res;
}

VAST_API VAST_INLINE vast_ui_result vast_ui_panel_begin(vast_ui_context *ctx, vast_ui_rect rect)
{
    vast_ui_result res;
    f32 s = ctx->scale;

    res.rect.x = rect.x * s;
    res.rect.y = rect.y * s;
    res.rect.w = rect.w * s;
    res.rect.h = rect.h * s;
    res.state = VAST_UI_STATE_IDLE;

    if (ctx->stack_ptr < VAST_UI_MAX_STACK)
    {
        vast_ui_rect *r = &ctx->stack[ctx->stack_ptr++];
        r->x = res.rect.x;
        r->y = res.rect.y;
        r->w = res.rect.w;
        r->h = res.rect.h;
        ctx->cursor_y = 0.0f;
    }

    return res;
}

VAST_API VAST_INLINE void vast_ui_panel_end(vast_ui_context *ctx)
{
    if (ctx->stack_ptr > 0)
    {
        ctx->stack_ptr--;
    }
}

VAST_API VAST_INLINE vast_ui_result vast_ui_drag_header(
    vast_ui_context *ctx, u32 id, f32 *win_x, f32 *win_y,
    f32 win_w, f32 win_h, f32 header_h)
{
    vast_ui_rect header_rect = vast_ui_rect_init(*win_x, *win_y, win_w, header_h);
    vast_ui_result res = vast_ui_internal_process(ctx, id, header_rect);

    if (res.state & VAST_UI_STATE_PRESSED)
    {
        ctx->dragged_window_id = id;
        ctx->drag_offset_x = (ctx->mouse_x / ctx->scale) - *win_x;
        ctx->drag_offset_y = (ctx->mouse_y / ctx->scale) - *win_y;
    }

    if (ctx->dragged_window_id == id && (res.state & VAST_UI_STATE_HELD))
    {
        f32 max_x = ((f32)ctx->window_width / ctx->scale) - win_w;
        f32 max_y = ((f32)ctx->window_height / ctx->scale) - win_h;

        if (max_x < 0.0f)
            max_x = 0.0f;
        if (max_y < 0.0f)
            max_y = 0.0f;

        *win_x = (ctx->mouse_x / ctx->scale) - ctx->drag_offset_x;
        *win_y = (ctx->mouse_y / ctx->scale) - ctx->drag_offset_y;

        if (*win_x < 0.0f)
            *win_x = 0.0f;
        if (*win_y < 0.0f)
            *win_y = 0.0f;
        if (*win_x > max_x)
            *win_x = max_x;
        if (*win_y > max_y)
            *win_y = max_y;

        res.rect.x = *win_x * ctx->scale;
        res.rect.y = *win_y * ctx->scale;
    }

    return res;
}

VAST_API VAST_INLINE vast_ui_result vast_ui_button(vast_ui_context *ctx, u32 id, vast_ui_rect rect)
{
    return vast_ui_internal_process(ctx, id, rect);
}

VAST_API VAST_INLINE void vast_ui_push_rect(vast_ui_context *ctx, vast_ui_result res, u32 rgba_color)
{
    if (ctx->instances_count < ctx->instances_size)
    {
        vast_ui_instance_data inst;
        inst.pos_x = res.rect.x;
        inst.pos_y = res.rect.y;
        inst.size_x = res.rect.w;
        inst.size_y = res.rect.h;
        inst.color = rgba_color;
        inst.glyph_index = VAST_UI_SOLID_FILL_MAGIC_INDEX;
        ctx->instances[ctx->instances_count++] = inst;
    }
}

VAST_API VAST_INLINE void vast_ui_push_text(vast_ui_context *ctx, s8 *text, vast_ui_rect rect, u32 rgba_color)
{
    u32 i = 0;
    f32 cursor_x = rect.x;
    f32 cursor_y = rect.y;
    f32 glyph_size = ctx->font_size * ctx->scale;
    f32 snapped_size = (f32)(i32)(glyph_size + 0.5f);
    f32 advance = glyph_size * ctx->font_advance_factor;

    while (text[i] != '\0' && ctx->instances_count < ctx->instances_size)
    {
        vast_ui_instance_data glyph;
        glyph.pos_x = (f32)(i32)(cursor_x + 0.5f);
        glyph.pos_y = (f32)(i32)(cursor_y + 0.5f);
        glyph.size_x = snapped_size;
        glyph.size_y = snapped_size;
        glyph.color = rgba_color;
        glyph.glyph_index = (u32)(u8)text[i];

        ctx->instances[ctx->instances_count++] = glyph;

        cursor_x += advance;
        i++;
    }
}

VAST_API VAST_INLINE void vast_ui_label(vast_ui_context *ctx, s8 *text, u32 rgba_color)
{
    f32 s = ctx->scale;
    f32 pad = ctx->padding * s;
    f32 scaled_glyph_size = ctx->font_size * s;
    vast_ui_rect rect;

    if (ctx->stack_ptr > 0)
    {
        vast_ui_rect *panel = &ctx->stack[ctx->stack_ptr - 1];
        f32 pad2 = pad * 2.0f;

        rect.x = panel->x + pad;
        rect.y = panel->y + ctx->cursor_y + pad;
        rect.w = panel->w - pad2;
        rect.h = scaled_glyph_size;

        ctx->cursor_y += rect.h + pad;
    }
    else
    {
        rect.x = 0.0f;
        rect.y = ctx->cursor_y;
        rect.w = 100.0f * s;
        rect.h = scaled_glyph_size;
        ctx->cursor_y += rect.h + pad;
    }

    vast_ui_push_text(ctx, text, rect, rgba_color);
}

/* #############################################################################
 * # [SECTION] UI Renderer
 * #############################################################################
 */
VAST_API void vast_ui_render(vast_platform_state *state, u8 *post_process_enabled)
{
    static u8 ui_initialized = 0;

    static vast_ui_context ui_ctx = {0};
    static vast_ui_shader ui_shader = {0};
    static u32 ui_vao = 0;
    static u32 ui_vbo = 0;
    static u32 font_texture = 0;
    static u32 font_texture_width = 1024;
    static u32 font_texture_height = 1024;

    vast_opengl_api *gl = &state->graphics.opengl;

    if (!ui_initialized)
    {
        u8 *shader_data_vertex = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/50_vast_ui.vs");
        u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "shaders/50_vast_ui.fs");
        u8 *font_data = (u8 *)vast_io_file_read_to_memory(state, &state->memory, "assets/font.raw");

#define INSTANCE_DATA_COUNT 1024
        static vast_ui_instance_data instance_data[INSTANCE_DATA_COUNT];

        if (!shader_data_vertex || !shader_data_fragment || !font_data)
        {
            state->memory.offset = 0;
            return;
        }

        /* Setup ui context */
        ui_ctx.instances = instance_data;
        ui_ctx.instances_count = 0;
        ui_ctx.instances_size = INSTANCE_DATA_COUNT;

        /* Load Shader */
        if (opengl_shader_load(&ui_shader.header, (s8 *)shader_data_vertex, (s8 *)shader_data_fragment, state->api.io_print, gl))
        {
            ui_shader.loc_iResolution = gl->glGetUniformLocation(ui_shader.header.program, "iResolution");
            ui_shader.loc_iTime = gl->glGetUniformLocation(ui_shader.header.program, "iTime");
            ui_shader.loc_texture_font = gl->glGetUniformLocation(ui_shader.header.program, "tex_font");
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
                      state->window.width, state->window.height,
                      (f32)state->input.mouse.x,
                      (f32)state->window.height - (f32)state->input.mouse.y,
                      state->input.mouse.keys_is_down[VAST_MOUSE_KEY_LEFT],
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

                u32 btn_color = *post_process_enabled ? 0xFF00AA00 : 0xFF0000AA;

                if (btn.state & VAST_UI_STATE_PRESSED)
                {
                    *post_process_enabled = !(*post_process_enabled);
                }

                vast_ui_push_rect(&ui_ctx, btn, btn_color);
                vast_ui_push_text(&ui_ctx, "Post Process (B)", btn.rect, 0xFFFFFFFF);
            }
        }

        /* UI Scaling */
        {
            vast_ui_result btn_minus = vast_ui_button(&ui_ctx, 3, vast_ui_rect_init(0.0f, 0.0f, 2 * font_size * 0.55f, font_size));
            vast_ui_result btn_plus = vast_ui_button(&ui_ctx, 4, vast_ui_rect_init(0.0f, 0.0f, 2 * font_size * 0.55f, font_size));

            u32 btn_minus_color = btn_minus.state & VAST_UI_STATE_HOVER ? 0xFF333344 : 0xFF181818;
            u32 btn_plus_color = btn_plus.state & VAST_UI_STATE_HOVER ? 0xFF333344 : 0xFF181818;

            if (btn_minus.state & VAST_UI_STATE_PRESSED)
            {
                ui_scale -= 0.1f;
                ui_scale = vast_maxf(0.5f, ui_scale);
            }

            if (btn_plus.state & VAST_UI_STATE_PRESSED)
            {
                ui_scale += 0.1f;
                ui_scale = vast_minf(2.0f, ui_scale);
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

            vast_sb_s8(&sb, "FPS  : ");
            vast_sb_i32_pad(&sb, (i32)state->timing.frame_rate, 8, ' ', VAST_SB_PAD_LEFT);
            vast_sb_s8(&sb, "\0");

            vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

            sb.length = 0;

            vast_sb_s8(&sb, "DELTA: ");
            vast_sb_f64_pad(&sb, (f64)state->timing.time_delta, 4, 8, ' ', VAST_SB_PAD_LEFT);
            vast_sb_s8(&sb, "\0");

            vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);

            sb.length = 0;

            vast_sb_s8(&sb, "MEM  : ");
            vast_sb_i32_pad(&sb, (i32)state->memory.size / 1024, 8, ' ', VAST_SB_PAD_LEFT);
            vast_sb_s8(&sb, " KB");
            vast_sb_s8(&sb, "\0");

            vast_ui_label(&ui_ctx, buffer, 0xFFFFFFFF);
        }

        vast_ui_panel_end(&ui_ctx);
        VAST_PROFILER_END(font_update);

        /* Profiler Metrics Window */
        {
            static f32 prof_win_w = 550.0f;
            static f32 prof_win_h = 300.0f;
            static f32 prof_win_x = 0.0f;

            f32 prof_win_y = (f32)state->window.height - (prof_win_h + prof_win_x);

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
                        sb.size = 256;
                        sb.length = 0;

                        if (i == 0)
                        {
                            vast_sb_s8_pad(&sb, "Name", 24, ' ', VAST_SB_PAD_RIGHT);
                            vast_sb_s8_pad(&sb, "last", 7, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8_pad(&sb, "min", 7, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8_pad(&sb, "max", 7, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8_pad(&sb, "avg", 7, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8_pad(&sb, "cnt", 6, ' ', VAST_SB_PAD_LEFT);
                            vast_sb_s8(&sb, "\0");

                            vast_ui_label(&ui_ctx, prof_buf, 0xFFAAAAAA);
                            sb.length = 0;
                        }

                        vast_sb_s8_pad(&sb, entry.name, 24, ' ', VAST_SB_PAD_RIGHT);
                        vast_sb_f64_pad(&sb, entry.time_ms_last, 2, 7, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_f64_pad(&sb, entry.time_ms_min, 2, 7, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_f64_pad(&sb, entry.time_ms_max, 2, 7, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_f64_pad(&sb, entry.counter > 0 ? (entry.time_ms_total / (f64)entry.counter) : 0.0, 2, 7, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_i32_pad(&sb, (i32)entry.counter, 6, ' ', VAST_SB_PAD_LEFT);
                        vast_sb_s8(&sb, "\0");

                        vast_ui_label(&ui_ctx, prof_buf, 0xFFFFFFFF);
                    }
                }
                else
                {
                    vast_ui_label(&ui_ctx, "No profiler entries found.", 0xFF888888);
                }
            }
            vast_ui_panel_end(&ui_ctx);
        }

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

        gl->glUniform3f(ui_shader.loc_iResolution, (f32)state->window.width, (f32)state->window.height, 1.0f);
        gl->glUniform1f(ui_shader.loc_iTime, (f32)state->timing.time_elapsed);
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