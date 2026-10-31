#define VAST_APPLICATION
#include "vast.h"

void vast_update(vast_platform_state *platform_state)
{
    vast_platform_input_keyboard *keyboard = &platform_state->input.keyboard;

    if (!platform_state->application_initialized)
    {
        platform_state->api.io_print("VAST - Virtual Adaptive Sparse Tiles\n");

        platform_state->window.title = "VAST - Virtual Adaptive Sparse Tiles (F1=Debug, V=Viz. Modes)\n";
        platform_state->window.title_changed = 1;

        platform_state->window.clear_color_r = 0.1f;
        platform_state->window.clear_color_g = 0.6f;
        platform_state->window.clear_color_b = 0.1f;
        platform_state->window.clear_color_changed = 1;

        platform_state->application_initialized = 1;
    }

    if (keyboard->keys_is_down[VAST_KEYBOARD_KEY_W])
    {
        platform_state->window.clear_color_r += 0.05f;
        platform_state->window.clear_color_changed = 1;
    }

    if (keyboard->keys_is_down[VAST_KEYBOARD_KEY_S])
    {
        platform_state->window.clear_color_r -= 0.05f;
        platform_state->window.clear_color_changed = 1;
    }
}

#ifdef _WIN32
#ifdef __clang__
#elif __GNUC__
__attribute((externally_visible))
#endif
i32 DllMainCRTStartup(void)
{
    return 1;
}
#endif