#ifndef VAST_UI_H
#define VAST_UI_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h" /* Basic types (u8, u32, ...)    */

/* #############################################################################
 * # [SECTION] Structs and Data
 * #############################################################################
 */
#ifndef VAST_UI_MAX_STACK
#define VAST_UI_MAX_STACK 8
#endif

#define VAST_UI_SOLID_FILL_MAGIC_INDEX 424242u

typedef struct vast_ui_instance_data
{
    f32 pos_x;       /* Screen X in pixels (or NDC) */
    f32 pos_y;       /* Screen Y in pixels */
    f32 size_x;      /* Glyph width in pixels */
    f32 size_y;      /* Glyph height in pixels */
    u32 color;       /* Packed RGBA8 (0xAABBGGRR) */
    u32 glyph_index; /* 0..255 index into the 16x16 atlas or SOLID_FILL_MAGIC */

} vast_ui_instance_data;

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

/* #############################################################################
 * # [SECTION] Immediate Mode API
 * #############################################################################
 */
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

VAST_API VAST_INLINE vast_ui_rect vast_ui_rect_init(f32 x, f32 y, f32 width, f32 height)
{
    vast_ui_rect result;

    result.x = x;
    result.y = y;
    result.w = width;
    result.h = height;

    return result;
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

        if (text[i] == ' ')
        {
            cursor_x += advance;
            i++;
            continue;
        }

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

#endif /* VAST_UI_H */