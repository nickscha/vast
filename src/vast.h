#ifndef VAST_H
#define VAST_H

#include "vast_types.h"

/* #############################################################################
 * # [SECTION] Platform Mouse Input
 * #############################################################################
 */
typedef enum vast_platform_input_mouse_keys
{
    /* basic mouse keys */
    VAST_MOUSE_KEY_LEFT,
    VAST_MOUSE_KEY_MIDDLE,
    VAST_MOUSE_KEY_RIGHT,

    /* padding */
    VAST_MOUSE_KEY_UNUSED1,
    VAST_MOUSE_KEY_UNUSED2,
    VAST_MOUSE_KEY_UNUSED3,
    VAST_MOUSE_KEY_UNUSED4,
    VAST_MOUSE_KEY_UNUSED5,

    VAST_MOUSE_KEYS_COUNT

} vast_platform_input_mouse_keys;

typedef struct vast_platform_input_mouse
{
    i32 dx;     /* Relative movement delta for x  */
    i32 dy;     /* Relative movement delta for y  */
    i32 x;      /* Mouse position on screen for x */
    i32 y;      /* Mouse position on screen for y */
    f32 scroll; /* Mouse scroll wheel delta       */

    u8 keys_is_down[VAST_MOUSE_KEYS_COUNT];
    u8 keys_was_down[VAST_MOUSE_KEYS_COUNT];

} vast_platform_input_mouse;

/* #############################################################################
 * # [SECTION] Platform Keyboard Input
 * #############################################################################
 */
typedef enum vast_platform_input_keyboard_keys
{
    /* numbers */
    VAST_KEYBOARD_KEY_0,
    VAST_KEYBOARD_KEY_1,
    VAST_KEYBOARD_KEY_2,
    VAST_KEYBOARD_KEY_3,
    VAST_KEYBOARD_KEY_4,
    VAST_KEYBOARD_KEY_5,
    VAST_KEYBOARD_KEY_6,
    VAST_KEYBOARD_KEY_7,
    VAST_KEYBOARD_KEY_8,
    VAST_KEYBOARD_KEY_9,

    /* letters */
    VAST_KEYBOARD_KEY_A,
    VAST_KEYBOARD_KEY_B,
    VAST_KEYBOARD_KEY_C,
    VAST_KEYBOARD_KEY_D,
    VAST_KEYBOARD_KEY_E,
    VAST_KEYBOARD_KEY_F,
    VAST_KEYBOARD_KEY_G,
    VAST_KEYBOARD_KEY_H,
    VAST_KEYBOARD_KEY_I,
    VAST_KEYBOARD_KEY_J,
    VAST_KEYBOARD_KEY_K,
    VAST_KEYBOARD_KEY_L,
    VAST_KEYBOARD_KEY_M,
    VAST_KEYBOARD_KEY_N,
    VAST_KEYBOARD_KEY_O,
    VAST_KEYBOARD_KEY_P,
    VAST_KEYBOARD_KEY_Q,
    VAST_KEYBOARD_KEY_R,
    VAST_KEYBOARD_KEY_S,
    VAST_KEYBOARD_KEY_T,
    VAST_KEYBOARD_KEY_U,
    VAST_KEYBOARD_KEY_V,
    VAST_KEYBOARD_KEY_W,
    VAST_KEYBOARD_KEY_X,
    VAST_KEYBOARD_KEY_Y,
    VAST_KEYBOARD_KEY_Z,

    /* function keys */
    VAST_KEYBOARD_KEY_F1,
    VAST_KEYBOARD_KEY_F2,
    VAST_KEYBOARD_KEY_F3,
    VAST_KEYBOARD_KEY_F4,
    VAST_KEYBOARD_KEY_F5,
    VAST_KEYBOARD_KEY_F6,
    VAST_KEYBOARD_KEY_F7,
    VAST_KEYBOARD_KEY_F8,
    VAST_KEYBOARD_KEY_F9,
    VAST_KEYBOARD_KEY_F10,
    VAST_KEYBOARD_KEY_F11,
    VAST_KEYBOARD_KEY_F12,

    /* control keys */
    VAST_KEYBOARD_KEY_CONTROL,
    VAST_KEYBOARD_KEY_RETURN,
    VAST_KEYBOARD_KEY_SPACE,
    VAST_KEYBOARD_KEY_SHIFT,
    VAST_KEYBOARD_KEY_TAB,
    VAST_KEYBOARD_KEY_ALT,
    VAST_KEYBOARD_KEY_ESCAPE,

    /* movement/arrow keys */
    VAST_KEYBOARD_KEY_LEFT,
    VAST_KEYBOARD_KEY_UP,
    VAST_KEYBOARD_KEY_RIGHT,
    VAST_KEYBOARD_KEY_DOWN,

    /* others */
    VAST_KEYBOARD_KEY_PLUS,
    VAST_KEYBOARD_KEY_MINUS,
    VAST_KEYBOARD_KEY_POINT,
    VAST_KEYBOARD_KEY_COMMA,

    /* padding */
    VAST_KEYBOARD_KEY_UNUSED3,

    VAST_KEYBORD_KEYS_COUNT

} vast_platform_input_keyboard_keys;

/*
    Key Pressed:   keys_is_down[VAST_KEYBOARD_KEY_A] && !keys_was_down[VAST_KEYBOARD_KEY_A]
    Key Released: !keys_is_down[VAST_KEYBOARD_KEY_A] &&  keys_was_down[VAST_KEYBOARD_KEY_A]
*/
typedef struct vast_platform_input_keyboard
{
    u8 keys_is_down[VAST_KEYBORD_KEYS_COUNT];
    u8 keys_was_down[VAST_KEYBORD_KEYS_COUNT];

} vast_platform_input_keyboard;

/* #############################################################################
 * # [SECTION] Platform Controller Input
 * #############################################################################
 */
typedef struct vast_platform_input_controller
{
    u8 connected;

    u8 button_a;
    u8 button_b;
    u8 button_x;
    u8 button_y;
    u8 shoulder_left;
    u8 shoulder_right;
    u8 trigger_left;
    u8 trigger_right;
    u8 dpad_up;
    u8 dpad_down;
    u8 dpad_left;
    u8 dpad_right;
    u8 stick_left;
    u8 stick_right;
    u8 start;
    u8 back;
    f32 stick_left_x;
    f32 stick_left_y;
    f32 stick_right_x;
    f32 stick_right_y;
    f32 trigger_left_value;
    f32 trigger_right_value;

} vast_platform_input_controller;

/* #############################################################################
 * # [SECTION] Platform Input
 * #############################################################################
 */
typedef struct vast_platform_input
{
    vast_platform_input_mouse mouse;
    vast_platform_input_keyboard keyboard;
    vast_platform_input_controller controller;

} vast_platform_input;

/* #############################################################################
 * # [SECTION] Platform Window
 * #############################################################################
 */
typedef struct vast_platform_window
{
    u8 size_changed;
    u8 clear_color_changed;
    u8 title_changed;

    u8 minimized;

    u32 width;
    u32 height;

    f32 clear_color_r;
    f32 clear_color_g;
    f32 clear_color_b;
    f32 clear_color_a;

    s8 *title;

} vast_platform_window;

/* #############################################################################
 * # [SECTION] Platform API
 * #############################################################################
 */
typedef u8 (*vast_platform_api_io_print)(s8 *string);
typedef u8 (*vast_platform_api_io_file_size)(s8 *filename, u32 *file_size);
typedef u8 (*vast_platform_api_io_file_read)(s8 *filename, u8 *buffer, u32 buffer_size);
typedef u8 (*vast_platform_api_io_file_write)(s8 *filename, u8 *buffer, u32 buffer_size);

typedef struct vast_platform_api
{
    vast_platform_api_io_print io_print;
    vast_platform_api_io_file_size io_file_size;
    vast_platform_api_io_file_read io_file_read;
    vast_platform_api_io_file_write io_file_write;

} vast_platform_api;

/* #############################################################################
 * # [SECTION] Platform Memory
 * #############################################################################
 */
typedef struct vast_platform_memory
{
    u8 *data;
    u32 size;

} vast_platform_memory;

/* #############################################################################
 * # [SECTION] Platform Timing
 * #############################################################################
 */
typedef struct vast_platform_timing
{
    i32 frame_count;       /* Frames processed count               */
    f64 frame_rate;        /* Frame Rate per second                */
    u32 frame_rate_target; /* Targeted Frame rate per second       */
    f64 frame_rate_raw;    /* Frame Rate per second raw (no cap)   */
    f64 time_elapsed;      /* Total elapsed time in seconds        */
    f64 time_delta;        /* Current render frame time in seconds */

} vast_platform_timing;

/* #############################################################################
 * # [SECTION] Main entry point (vast_update)
 * #############################################################################
 */
typedef struct vast_platform_state
{
    vast_platform_api api;       /* Platform provided functions */
    vast_platform_window window; /* Platform window information */
    vast_platform_input input;   /* Platform input */
    vast_platform_memory memory; /* Platform memory */
    vast_platform_timing timing; /* Platform timing */

    u8 running;
    u8 application_initialized;

} vast_platform_state;

#ifdef VAST_APPLICATION
void vast_update(vast_platform_state *platform_state);
#else

VAST_API VAST_INLINE void vast_update_stub(vast_platform_state *platform_state)
{
    platform_state->api.io_print("[vast][error] No 'vast_update' function has been set! Using 'vast_update_stub'!\n");
    platform_state->api.io_print("[vast][error] Define the following function in your code:\n\n");
    platform_state->api.io_print("  u8 vast_update(vast_platform_state *platform_state)\n  {\n    /* your code */\n  }\n\n");
    platform_state->api.io_print("[vast][error]\n");

    platform_state->window.clear_color_r = 0.9f;
    platform_state->window.clear_color_g = 0.1f;
    platform_state->window.clear_color_b = 0.1f;
    platform_state->window.clear_color_changed = 1;

    platform_state->window.title = "[vast][error] No 'vast_update' function has been set! Using 'vast_update_stub'!\n";
    platform_state->window.title_changed = 1;
}

typedef void (*vast_update_function)(vast_platform_state *platform_state);

static vast_update_function vast_update = vast_update_stub;

#endif

#endif /* VAST_H */
