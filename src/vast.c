#define VAST_APPLICATION
#include "vast.h"
#include "vast_state.h"

void vast_update(vast_platform_state *platform_state)
{
    vast_state *state = (vast_state *)platform_state->memory.data;
    vast_platform_window *window = &platform_state->window;
    vast_platform_input_keyboard *keyboard = &platform_state->input.keyboard;

    if (!platform_state->application_initialized)
    {
        platform_state->api.io_print("VAST - Virtual Adaptive Sparse Tiles\n");

        window->title = "VAST - Virtual Adaptive Sparse Tiles (F1=Show/Hide UI, V=Viz. Modes, F9=Borderless, F11=Fullscreen)\n";
        window->title_changed = 1;

        window->clear_color_r = 0.1f;
        window->clear_color_g = 0.6f;
        window->clear_color_b = 0.1f;
        window->clear_color_changed = 1;

        /* Assign memory regions */
        {
            u32 state_struct_size = (u32)sizeof(vast_state);
            u8 *base = (u8 *)platform_state->memory.data;

            /* Permanent */
            state->memory_region_permanent.data = base;
            state->memory_region_permanent.offset = state_struct_size;
            state->memory_region_permanent.size = state_struct_size;

            /* Transient */
            state->memory_region_transient.data = base + state_struct_size;
            state->memory_region_transient.offset = 0;
            state->memory_region_transient.size = (platform_state->memory.size > state_struct_size) ? (platform_state->memory.size - state_struct_size) : 0;

            /* Mark Platform Memory as consumed */
            platform_state->memory.offset = platform_state->memory.size;
            platform_state->application_initialized = 1;
        }

        platform_state->application_initialized = 1;
    }

    /* Change window mode */
    if (keyboard->keys_is_down[VAST_KEYBOARD_KEY_F9] && !keyboard->keys_was_down[VAST_KEYBOARD_KEY_F9])
    {
        window->window_mode = window->window_mode > 0 ? VAST_PLATFORM_WINDOW_MODE_WINDOWED : VAST_PLATFORM_WINDOW_MODE_BORDERLESS;
        window->window_mode_changed = 1;
    }

    if (keyboard->keys_is_down[VAST_KEYBOARD_KEY_F11] && !keyboard->keys_was_down[VAST_KEYBOARD_KEY_F11])
    {
        window->window_mode = window->window_mode > 0 ? VAST_PLATFORM_WINDOW_MODE_WINDOWED : VAST_PLATFORM_WINDOW_MODE_FULLSCREEN;
        window->window_mode_changed = 1;
    }

    /* Reset transient memory after every update */
    state->memory_region_transient.offset = 0;
}
