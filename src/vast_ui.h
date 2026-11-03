#ifndef VAST_UI_H
#define VAST_UI_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"          /* Basic types (u8, u32, ...)  */
#include "vast.h"                /* Platform State              */
#include "vast_font.h"           /* Font Bitmap data            */
#include "vast_string_builder.h" /* String parsing              */
#include "vast_profiler.h"       /* Profiler metrics to display */
#include "vast_opengl.h"         /* OpenGL functions            */

/* #############################################################################
 * # [SECTION] Structs and Data
 * #############################################################################
 */
typedef enum vast_ui_glyph_state_flag
{
    GLYPH_STATE_NONE = 0,
    GLYPH_STATE_BLINK = 1 << 8,
    GLYPH_STATE_VFLIP = 1 << 9,
    GLYPH_STATE_HFLIP = 1 << 10

} vast_ui_glyph_state_flag;

typedef struct vast_ui_glyph
{
    u16 x;
    u16 y;
    u16 packed_glyph_state; /* packs the glyph_index | glyph_state_flag */
    u16 packed_color;       /* rgb565_color */
} vast_ui_glyph;

typedef struct vast_ui_shader
{
    shader_header header;

    i32 loc_iResolution;
    i32 loc_iTime;
    i32 loc_iTextureInfo;
    i32 loc_iTexture;
    i32 loc_iFontScale;

} vast_ui_shader;

static s8 *vast_ui_shader_vertex =
    "#version 330 core\n"
    "layout(location=0)in vec2 p;"
    "layout(location=1)in uvec4 g;"
    "uniform vec3 r;"
    "uniform vec4 t;"
    "uniform float s;"
    "out vec2 vUV;"
    "out vec3 vC;"
    "flat out uint b;"
    "void main(){"
    "float i=g.z&255u,C=t.x/t.z;"
    "b=(g.z>>8)&1u;"
    "float v=(g.z>>9)&1u,h=(g.z>>10)&1u;"
    "vec2 Q=vec2(p.x*(1.-2.*h)+h,p.y*(1.-2.*v)+v);"
    "vec2 p2=vec2(g.xy)+Q*t.zw*s;"
    "vec2 N=p2/r.xy*2.-1.;"
    "gl_Position=vec4(N.x,-N.y,0,1);"
    "vUV=vec2((mod(i,C)+p.x)*t.z/t.x,(floor(i/C)+p.y)*t.w/t.y);"
    "uint K=g.w;"
    "vC=vec3((K>>11&31u)/31.,(K>>5&63u)/63.,(K&31u)/31.);"
    "}";

static s8 *vast_ui_shader_fragment =
    "#version 330 core\n"
    "in vec2 vUV;"
    "in vec3 vC;"
    "flat in uint b;"
    "uniform sampler2D iTexture;"
    "uniform float iTime;"
    "out vec4 F;"
    "void main(){"
    "float a=texture(iTexture,vUV).r;"
    "if(b==1u&&fract(iTime)<.5)a=0.;" /* Blink Support */
    "F=vec4(vC,a);"
    "}";

/* #############################################################################
 * # [SECTION] Font & Glyph loading and parsing
 * #############################################################################
 */
VAST_API VAST_INLINE u16 vast_ui_pack_rgb565(u8 r, u8 g, u8 b)
{
    return (u16)(((r >> 3) << 11) |
                 ((g >> 2) << 5) |
                 ((b >> 3) << 0));
}

VAST_API VAST_INLINE u16 vast_ui_glyph_advance_x(f32 font_scale)
{
    return (u16)((f32)(VAST_FONT_GLYPH_WIDTH + 1) * font_scale);
}

VAST_API VAST_INLINE u16 vast_ui_glyph_advance_y(f32 font_scale)
{
    return (u16)(((f32)VAST_FONT_GLYPH_HEIGHT * font_scale) + (f32)VAST_FONT_GLYPH_HEIGHT);
}

VAST_API void vast_ui_glyph_add(
    vast_ui_glyph *glyph_buffer,    /* pre allocated glyph array */
    u32 glyph_buffer_size,          /* maximum size */
    u32 *glyph_buffer_count,        /* current size */
    s8 *text,                       /* text to append */
    u16 *offset_x,                  /* current offset x. Note: this is advanced by this function */
    u16 *offset_y,                  /* current offset y. Note: this is advanced by this function */
    u16 color_rgb565,               /* rgb565 packed color. Use: pack_rgb565(255, 255, 255); */
    vast_ui_glyph_state_flag state, /* GLYPH_STATE_BLINK | GLYPH_STATE_VFLIP | GLYPH_STATE_HFLIP */
    f32 font_scale)
{
    u16 x = *offset_x;
    u16 y = *offset_y;

    u16 start_x = x;
    u16 advance_x = vast_ui_glyph_advance_x(font_scale);
    u16 advance_y = vast_ui_glyph_advance_y(font_scale);

    u32 count = *glyph_buffer_count;
    vast_ui_glyph *dst = glyph_buffer;

    while (*text && count < glyph_buffer_size)
    {
        s8 c = *text++;

        if (c == '\n')
        {
            x = start_x;
            y = (u16)(y + advance_y);
            continue;
        }

        if (c == ' ')
        {
            x = (u16)(x + advance_x);
            continue;
        }

        {
            i32 glyph_index = vast_font_s8_to_index(c);

            if (glyph_index >= 0)
            {
                dst[count].x = x;
                dst[count].y = y;
                dst[count].packed_glyph_state =
                    (u16)glyph_index | (u16)state;
                dst[count].packed_color = color_rgb565;

                ++count;
                x = (u16)(x + advance_x);
            }
        }
    }

    *glyph_buffer_count = count;
    *offset_x = x;
    *offset_y = y;
}

/* #############################################################################
 * # [SECTION] UI Setup & Rendering
 * #############################################################################
 */
VAST_API void vast_ui_render(
    vast_platform_state *state,
    i32 thread_count,
    i32 handle_count,
    i32 working_set, i32 peak_working_set, i32 private_bytes,
    s8 *gl_version,
    s8 *gl_renderer,
    s8 *gl_vendor,
    u8 borderless_enabled,
    u8 fullscreen_enabled,
    u8 shader_paused,
    u8 screen_recording_enabled)
{
    static u8 ui_initialized = 0;
    static u32 ui_font_texture_id;
    static vast_ui_shader font_shader = {0};
    static u32 font_vao;
    static u32 glyph_vbo;

#define GLYPH_BUFFER_SIZE 4192
    static vast_ui_glyph glyph_buffer[GLYPH_BUFFER_SIZE];
    static u32 glyph_buffer_count = 0; /* Total glyph count */
    static u32 glyph_count_static = 0; /* Static glyphs only need to be buffered once */
    static u16 offset_x = 0;
    static u16 offset_y = 0;
    static u8 glyph_initialized = 0;
    static s8 tmp[128];
    static f32 font_scale = 2.0f;
    static u16 advance_x;
    static u16 advance_y;

    if (!ui_initialized)
    {
        advance_x = vast_ui_glyph_advance_x(font_scale);
        advance_y = vast_ui_glyph_advance_y(font_scale);

        /* Generate font texture */
        {
            u8 vast_ui_font_pixels[VAST_FONT_WIDTH * VAST_FONT_HEIGHT];

            /* OpenGL does not allow 1bit packed texture data so we convert each bit to 1 byte */
            vast_font_unpack_1bit_to_8bit(vast_ui_font_pixels, vast_font, VAST_FONT_WIDTH, VAST_FONT_HEIGHT);

            glGenTextures(1, &ui_font_texture_id);
            glBindTexture(GL_TEXTURE_2D, ui_font_texture_id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, VAST_FONT_WIDTH, VAST_FONT_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, vast_ui_font_pixels);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glActiveTexture(GL_TEXTURE0);
        }

        /* Load font shader */
        {
            if (opengl_shader_load(&font_shader.header, vast_ui_shader_vertex, vast_ui_shader_fragment, state->api.io_print))
            {
                font_shader.loc_iResolution = glGetUniformLocation(font_shader.header.program, "r");
                font_shader.loc_iTextureInfo = glGetUniformLocation(font_shader.header.program, "t");
                font_shader.loc_iFontScale = glGetUniformLocation(font_shader.header.program, "s");
                font_shader.loc_iTime = glGetUniformLocation(font_shader.header.program, "iTime");
                font_shader.loc_iTexture = glGetUniformLocation(font_shader.header.program, "iTexture");
            }
        }

        /* Setup Vertex Buffer Object */
        {
            static f32 quad_vertices[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
            static u32 quad_vbo;

            glGenVertexArrays(1, &font_vao);
            glBindVertexArray(font_vao);

            glGenBuffers(1, &quad_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

            /* pos (location = 0) */
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), VAST_NULL);
            glVertexAttribDivisor(0, 0);

            glGenBuffers(1, &glyph_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);

            /* iGlyph (location = 1) */
            glEnableVertexAttribArray(1);
            glVertexAttribIPointer(1, 4, GL_UNSIGNED_SHORT, sizeof(vast_ui_glyph), VAST_NULL);
            glVertexAttribDivisor(1, 1);
        }

        /* Setup Fixed Shader Uniform constants */
        glUseProgram(font_shader.header.program);
        glUniform4f(font_shader.loc_iTextureInfo, VAST_FONT_WIDTH, VAST_FONT_HEIGHT, VAST_FONT_GLYPH_WIDTH, VAST_FONT_GLYPH_HEIGHT);
        glUniform1i(font_shader.loc_iTexture, 0);
        glUniform1f(font_shader.loc_iFontScale, font_scale);

        ui_initialized = 1;
    }

    /* Calculate Glpyhs */
    {
        u16 default_color = vast_ui_pack_rgb565(255, 255, 255);
        u16 separator_start_col = 11;
        u16 text_start_col = separator_start_col + 2;
        u16 offset_x_start = 10;
        u16 offset_y_start = 10;

        vast_sb t = {0};
        t.size = sizeof(tmp);
        t.buffer = tmp;

        if (!glyph_initialized)
        {
            u32 i = 0;

            offset_x = offset_x_start;
            offset_y = offset_y_start;

            vast_ui_glyph_add(
                glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count,
                "STATE\n"
                "FPS\n"
                "FPS TARGET\n"
                "FPS RAW\n"
                "FRAME\n"
                "DELTA\n"
                "TIME\n"
                "MOUSE X/Y\n"
                "MOUSE DX/DY\n"
                "SIZE X/Y\n"
                "THREADS\n"
                "HANDLES\n"
                "MEM WORK\n"
                "MEM PEAK\n"
                "MEM COMMIT\n"
                "GL VENDOR\n"
                "GL RENDER\n"
                "GL VERSION\n"
                "CONTROLLER\n"
                "STICK L X/Y\n"
                "STICK R X/Y\n"
                "TRIGGER L/R\n"
                "BUTTONS\n"
                "GL ERROR FS\n",
                &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);

            offset_x = (u16)(offset_x_start + advance_x * separator_start_col);
            offset_y = offset_y_start;

            /* Vertical ":" separator*/
            for (i = 0; i < 24; ++i)
            {

                if (i == 15)
                {
                    u16 offset_x_new = (u16)(offset_x_start + advance_x * text_start_col);
                    vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, gl_vendor, &offset_x_new, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
                }
                else if (i == 16)
                {
                    u16 offset_x_new = (u16)(offset_x_start + advance_x * text_start_col);
                    vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, gl_renderer, &offset_x_new, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
                }
                else if (i == 17)
                {
                    u16 offset_x_new = (u16)(offset_x_start + advance_x * text_start_col);
                    vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, gl_version, &offset_x_new, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
                }

                vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, ":\n", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
            }

            glyph_count_static = glyph_buffer_count;
            glyph_initialized = 1;
        }

        offset_x = (u16)(offset_x_start + advance_x * text_start_col);
        offset_y = offset_y_start;

        glyph_buffer_count = glyph_count_static;

        if (borderless_enabled)
        {
            vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "BORDERLESS ", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
        }
        else if (fullscreen_enabled)
        {
            vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "FULLSCREEN ", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
        }
        else
        {
            vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "WINDOWED ", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
        }
        if (shader_paused)
        {
            vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "PAUSED ", &offset_x, &offset_y, default_color, GLYPH_STATE_NONE, font_scale);
        }
        if (screen_recording_enabled)
        {
            vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "RECORDING ", &offset_x, &offset_y, vast_ui_pack_rgb565(255, 0, 0), GLYPH_STATE_BLINK, font_scale);
        }

#define CALC_GLYPH(txt, color)                                                                                                                          \
    do                                                                                                                                                  \
    {                                                                                                                                                   \
        offset_x = (u16)(offset_x_start + advance_x * text_start_col);                                                                                  \
        offset_y = (u16)(offset_y + advance_y);                                                                                                         \
        vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, txt.buffer, &offset_x, &offset_y, color, GLYPH_STATE_NONE, font_scale); \
        txt.length = 0;                                                                                                                                 \
    } while (0)

        vast_sb_f64(&t, state->timing.frame_rate, 2);
        CALC_GLYPH(t, default_color);

        /* Control Target FPS with arrow keys (0 = unlimited) */
        {
            u32 txt_length_temp = 0;

            if (state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_LEFT] &&
                !state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_LEFT])
            {
                state->timing.frame_rate_target -= state->timing.frame_rate_target < 10 ? state->timing.frame_rate_target : 10;
            }

            if (state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_RIGHT] &&
                !state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_RIGHT])
            {
                state->timing.frame_rate_target += 10;
            }

            offset_x = (u16)(offset_x_start + advance_x * (text_start_col - 1));
            offset_y = (u16)(offset_y + advance_y);
            vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, "<", &offset_x, &offset_y, vast_ui_pack_rgb565(255, 0, 0), GLYPH_STATE_BLINK, font_scale);
            offset_y = (u16)(offset_y - advance_y);

            vast_sb_f64(&t, state->timing.frame_rate_target, 2);
            txt_length_temp = t.length;
            CALC_GLYPH(t, default_color);

            offset_x = (u16)(offset_x_start + advance_x * (text_start_col + txt_length_temp));
            vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, ">", &offset_x, &offset_y, vast_ui_pack_rgb565(0, 255, 0), GLYPH_STATE_BLINK | GLYPH_STATE_HFLIP, font_scale);
        }

        vast_sb_f64(&t, state->timing.frame_rate_raw, 2);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, state->timing.frame_count);
        CALC_GLYPH(t, default_color);
        vast_sb_f64(&t, state->timing.time_delta, 6);
        CALC_GLYPH(t, default_color);
        vast_sb_f64(&t, state->timing.time_elapsed, 6);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, state->input.mouse.x);
        vast_sb_s8(&t, "/");
        vast_sb_i32(&t, state->input.mouse.y);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, state->input.mouse.dx);
        vast_sb_s8(&t, "/");
        vast_sb_i32(&t, state->input.mouse.dy);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, (i32)state->window.width);
        vast_sb_s8(&t, "/");
        vast_sb_i32(&t, (i32)state->window.height);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, thread_count);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, handle_count);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, working_set);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, peak_working_set);
        CALC_GLYPH(t, default_color);
        vast_sb_i32(&t, private_bytes);
        CALC_GLYPH(t, default_color);

        offset_y = (u16)(offset_y + advance_y * 3);

        if (state->input.controller.connected)
        {
            vast_platform_input_controller *controller = &state->input.controller;

            vast_sb_s8(&t, "CONNECTED");
            CALC_GLYPH(t, vast_ui_pack_rgb565(0, 255, 0));
            vast_sb_f64(&t, (f64)controller->stick_left_x, 6);
            vast_sb_s8(&t, "/");
            vast_sb_f64(&t, (f64)controller->stick_left_y, 6);
            CALC_GLYPH(t, default_color);
            vast_sb_f64(&t, (f64)controller->stick_right_x, 6);
            vast_sb_s8(&t, "/");
            vast_sb_f64(&t, (f64)controller->stick_right_y, 6);
            CALC_GLYPH(t, default_color);
            vast_sb_f64(&t, (f64)controller->trigger_left_value, 6);
            vast_sb_s8(&t, "/");
            vast_sb_f64(&t, (f64)controller->trigger_right_value, 6);
            CALC_GLYPH(t, default_color);

            vast_sb_s8(&t, "");

            /* clang-format off */
          if (controller->button_a)       vast_sb_s8(&t, "A ");
          if (controller->button_b)       vast_sb_s8(&t, "B ");
          if (controller->button_x)       vast_sb_s8(&t, "X ");
          if (controller->button_y)       vast_sb_s8(&t, "Y ");
          if (controller->dpad_left)      vast_sb_s8(&t, "DLEFT ");
          if (controller->dpad_right)     vast_sb_s8(&t, "DRIGHT ");
          if (controller->dpad_up)        vast_sb_s8(&t, "DUP ");
          if (controller->dpad_down)      vast_sb_s8(&t, "DDOWN ");
          if (controller->stick_left)     vast_sb_s8(&t, "LSTICK ");
          if (controller->stick_right)    vast_sb_s8(&t, "RSTICK ");
          if (controller->shoulder_left)  vast_sb_s8(&t, "LSHOULDER ");
          if (controller->shoulder_right) vast_sb_s8(&t, "RSHOULDER ");
            /* clang-format on */

            CALC_GLYPH(t, default_color);
        }
        else
        {
            vast_sb_s8(&t, "NOT FOUND");
            CALC_GLYPH(t, vast_ui_pack_rgb565(255, 165, 0));

            offset_y = (u16)(offset_y + advance_y * 4);
        }

        vast_sb_s8(&t, "NONE");
        CALC_GLYPH(t, vast_ui_pack_rgb565(0, 255, 0));

#undef CALC_GLYPH

        /* Show profiler entries */
        {
            u16 offset_memory_x = 300;
            u16 offset_memory_y = 10;
            u32 i;

            for (i = 0; i < vast_profiler_entries_count; ++i)
            {
                vast_profiler_entry entry = vast_profiler_entries[i];

                t.length = 0;

                if (i == 0)
                {
                    vast_sb_s8_pad(&t, " ", 25, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8_pad(&t, "last", 10, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8_pad(&t, "min", 10, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8_pad(&t, "max", 10, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8_pad(&t, "avg", 10, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8_pad(&t, "total", 10, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8_pad(&t, "count", 8, ' ', VAST_SB_PAD_LEFT);
                    vast_sb_s8(&t, "\n");

                    vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, t.buffer, &offset_memory_x, &offset_memory_y, vast_ui_pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
                    t.length = 0;
                }

                vast_sb_s8_pad(&t, entry.name, 23, ' ', VAST_SB_PAD_RIGHT);
                vast_sb_s8(&t, ": ");
                vast_sb_f64_pad(&t, entry.time_ms_last, 4, 10, ' ', VAST_SB_PAD_LEFT);
                vast_sb_f64_pad(&t, entry.time_ms_min, 4, 10, ' ', VAST_SB_PAD_LEFT);
                vast_sb_f64_pad(&t, entry.time_ms_max, 4, 10, ' ', VAST_SB_PAD_LEFT);
                vast_sb_f64_pad(&t, entry.time_ms_total / (f64)entry.counter, 4, 10, ' ', VAST_SB_PAD_LEFT);
                vast_sb_f64_pad(&t, entry.time_ms_total, 4, 10, ' ', VAST_SB_PAD_LEFT);
                vast_sb_i32_pad(&t, (i32)entry.counter, 8, ' ', VAST_SB_PAD_LEFT);
                vast_sb_s8(&t, "\n");

                vast_ui_glyph_add(glyph_buffer, GLYPH_BUFFER_SIZE, &glyph_buffer_count, t.buffer, &offset_memory_x, &offset_memory_y, vast_ui_pack_rgb565(255, 255, 255), GLYPH_STATE_NONE, font_scale);
            }
        }
    }

    /* Rendering */
    VAST_PROFILER_BEGIN(ui_render);

    glUseProgram(font_shader.header.program);
    glUniform3f(font_shader.loc_iResolution, (f32)state->window.width, (f32)state->window.height, 1.0f);
    glUniform1f(font_shader.loc_iTime, (f32)state->timing.time_elapsed);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(font_vao);
    glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
    glBufferData(GL_ARRAY_BUFFER, (i32)(glyph_buffer_count * sizeof(vast_ui_glyph)), glyph_buffer, GL_STREAM_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ui_font_texture_id);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, glyph_buffer_count);
    glBindVertexArray(0);

    glDisable(GL_BLEND);

    VAST_PROFILER_END(ui_render);
}

#endif /* VAST_UI_H */