#define VAST_APPLICATION
#include "vast.h"
#include "vast_state.h"

#include "vast_tile_renderer.h"
#include "vast_sky_renderer.h"
#include "vast_ui_renderer.h"
#include "vast_post_renderer.h"

/* TODO(nickscha): Unclean !!! */
static vast_platform_api *api;

#ifndef VAST_RELEASE
VAST_API f64 vast_profiler_time_ms(void)
{
    return api->time_ms_current();
}
#endif

VAST_API VAST_INLINE void vast_update_movement_camera(vast_platform_state *platform_state, vast_state *state)
{
    static u8 mouse_attached = 0;

    vast_camera *camera = &state->camera;
    f32 cam_speed = state->camera_speed * (f32)platform_state->timing.time_delta;

    vast_platform_input_keyboard *kb = &platform_state->input.keyboard;
    vast_platform_input_mouse *mouse = &platform_state->input.mouse;

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_W])
    {
        camera->position = vast_vec3_add(camera->position, vast_vec3_mulf(camera->forward, cam_speed));
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_S])
    {
        camera->position = vast_vec3_sub(camera->position, vast_vec3_mulf(camera->forward, cam_speed));
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_A])
    {
        camera->position = vast_vec3_sub(camera->position, vast_vec3_mulf(camera->right, cam_speed));
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_D])
    {
        camera->position = vast_vec3_add(camera->position, vast_vec3_mulf(camera->right, cam_speed));
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_SPACE])
    {
        camera->position = vast_vec3_add(camera->position, vast_vec3_mulf(camera->worldUp, cam_speed));
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_CONTROL])
    {
        camera->position = vast_vec3_sub(camera->position, vast_vec3_mulf(camera->worldUp, cam_speed));
    }

    /* Roll handling */
    if (kb->keys_is_down[VAST_KEYBOARD_KEY_Q])
    {
        camera->roll -= 1.0f;
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_E])
    {
        camera->roll += 1.0f;
    }

    /* FOV handling */
    if (kb->keys_is_down[VAST_KEYBOARD_KEY_F] && !kb->keys_was_down[VAST_KEYBOARD_KEY_F])
    {
        camera->fov = 90.0f;
        camera->roll = 0.0f;
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_PLUS])
    {
        camera->fov -= 1.0f;

        if (camera->fov <= 0.0f)
        {
            camera->fov = 1.0f;
        }
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_MINUS])
    {
        camera->fov += 1.0f;

        if (camera->fov >= 180.0f)
        {
            camera->fov = 179.0f;
        }
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_LEFT])
    {
        camera->yaw -= vast_minf(cam_speed * 0.1f, 89.0f);
    }

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_RIGHT])
    {
        camera->yaw += vast_minf(cam_speed * 0.1f, 89.0f);
    }

    /* Mouse handling */
    if (mouse->keys_is_down[VAST_MOUSE_KEY_RIGHT] && !mouse->keys_was_down[VAST_MOUSE_KEY_RIGHT])
    {
        mouse_attached = !mouse_attached;
    }

    if (mouse_attached)
    {
        camera->yaw += vast_minf((f32)mouse->dx * state->mouse_sensitivity, 89.0f);
        camera->pitch += vast_maxf((f32)mouse->dy * state->mouse_sensitivity, -89.0f);
        camera->pitch = vast_clampf(camera->pitch, -89.0f, 89.0f);

        if (mouse->scroll != 0.0f)
        {
            camera->fov = vast_clampf(camera->fov - (mouse->scroll * 2), 1.0f, 179.0f);
        }
    }

    vast_camera_update(camera);
}

VAST_API VAST_INLINE void vast_update_movement_sun(vast_platform_state *platform_state, vast_state *state)
{
    static u8 move_sun = 0;
    static u8 move_sun_loop = 0;
    static f32 move_val = 0.0f;

    vast_vec3 sun_dir;

    vast_platform_input_keyboard *kb = &platform_state->input.keyboard;

    if (kb->keys_is_down[VAST_KEYBOARD_KEY_M] && !kb->keys_was_down[VAST_KEYBOARD_KEY_M])
    {
        move_sun = !move_sun;

        if (move_sun)
        {
            move_val = 0.0f;
            move_sun_loop = 0;
        }
    }

    sun_dir = vast_vec3_init(0.0f, 0.0f, -1.0f);

    if (move_sun)
    {
        f32 time = (f32)platform_state->timing.time_elapsed * 0.5f;
        f32 delta = (f32)platform_state->timing.time_delta;

        if (kb->keys_is_down[VAST_KEYBOARD_KEY_L] && !kb->keys_was_down[VAST_KEYBOARD_KEY_L])
        {
            move_sun_loop = !move_sun_loop;
        }

        if (kb->keys_is_down[VAST_KEYBOARD_KEY_UP])
        {
            move_val += delta;
        }

        if (kb->keys_is_down[VAST_KEYBOARD_KEY_DOWN])
        {
            move_val -= delta;

            if (move_val < 0.0f)
            {
                move_val = 0.0f;
            }
        }

        sun_dir = vast_vec3_init(0.0f, vast_sinf(move_val), -vast_cosf(move_val));

        if (move_sun_loop)
        {
            sun_dir = vast_vec3_init(0.0f, vast_sinf(time * 0.025f), -vast_cosf(time * 0.025f));
        }
    }

    state->sun_direction = sun_dir;
}

VAST_API VAST_INLINE void vast_scene(vast_platform_state *platform_state, vast_state *state)
{
    static u8 scene_initialized = 0;

    if (!scene_initialized)
    {

        /* Camera Setup */
        state->camera = vast_camera_init();
        state->camera.position.x = -250.0f;
        state->camera.position.y = 250.0f;
        state->camera.position.z = -200.0f;
        state->camera_speed = 100.0f;

        /* Mouse setup */
        state->mouse_sensitivity = 0.1f;

        scene_initialized = 1;
    }

    /* Update camera */
    vast_update_movement_camera(platform_state, state);

    /* Sun/Moon movement logic */
    vast_update_movement_sun(platform_state, state);

    /* Reset draw calls accumulators */
    state->rendering_draw_calls_terrain = 0;
    state->rendering_draw_calls_water = 0;
    state->rendering_draw_calls_sky = 0;
    state->rendering_draw_calls_post = 0;
    state->rendering_draw_calls_ui = 0;

    /* Updated matrices*/
    state->projection = vast_mat4x4_perspective(VAST_DEG_TO_RAD(state->camera.fov), (f32)platform_state->window.width / (f32)platform_state->window.height, 0.1f, 8192.0f);
    state->view = vast_mat4x4_look_at(state->camera.position, vast_vec3_add(state->camera.position, state->camera.forward), state->camera.up);
    state->view_projection = vast_mat4x4_mul(state->projection, state->view);
    state->view_frustum = vast_frustum_extract(state->view_projection);

    /******************************/
    /* Application Rendering      */
    /******************************/
    {
        /* Post Processing shader */
        if (state->rendering_enabled_post)
        {
            vast_post_render_begin(platform_state, state);
        }

        platform_state->graphics.opengl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vast_tile_render(platform_state, state);

        if (state->rendering_enabled_sky)
        {
            vast_sky_render(platform_state, state);
        }

        /* Post Processing shader */
        if (state->rendering_enabled_post)
        {
            vast_post_render_end(platform_state, state);
        }

        /* UI Rendering should be untouched by post processing for now */
        if (state->rendering_enabled_ui)
        {
            vast_ui_render(platform_state, state);
        }
    }
}

void vast_update(vast_platform_state *platform_state)
{
    vast_state *state = (vast_state *)platform_state->memory.data;
    vast_platform_window *window = &platform_state->window;
    vast_platform_input_keyboard *keyboard = &platform_state->input.keyboard;

    api = &platform_state->api;

    (void)vast_sb_f64;
    (void)vast_sb_s8_pad;

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
            u32 perm_additional_size = 1024 * 1024 * 2;
            u32 perm_total_size = state_struct_size + perm_additional_size;
            u8 *base = (u8 *)platform_state->memory.data;

            if (!base || platform_state->memory.size < perm_total_size)
            {
                platform_state->api.io_print("[ERROR] Insufficient platform memory for vast_state!\n");
                return;
            }

            /* Permanent region starts right after vast_state struct */
            state->memory_region_permanent.data = base + state_struct_size;
            state->memory_region_permanent.offset = state_struct_size;
            state->memory_region_permanent.size = perm_additional_size;

            /* Transient region takes the remainder of platform memory */
            state->memory_region_transient.data = base + perm_total_size;
            state->memory_region_transient.offset = 0;
            state->memory_region_transient.size = platform_state->memory.size - perm_total_size;

            /* Mark Platform Memory as consumed */
            platform_state->memory.offset = platform_state->memory.size;
        }

        /* Set state defaults */
        state->rendering_enabled_terrain = 1;
        state->rendering_enabled_water = 1;
        state->rendering_enabled_sky = 1;
        state->rendering_enabled_post = 1;
        state->rendering_enabled_ui = 1;

        platform_state->application_initialized = 1;
    }

    /* Enable Disable Post Processing */
    if (keyboard->keys_is_down[VAST_KEYBOARD_KEY_B] && !keyboard->keys_was_down[VAST_KEYBOARD_KEY_B])
    {
        state->rendering_enabled_post = !state->rendering_enabled_post;
    }

    /* Enable Disable UI */
    if (keyboard->keys_is_down[VAST_KEYBOARD_KEY_F1] && !keyboard->keys_was_down[VAST_KEYBOARD_KEY_F1])
    {
        state->rendering_enabled_ui = !state->rendering_enabled_ui;
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

    vast_scene(platform_state, state);

    /* Reset transient memory after every update */
    state->memory_region_transient.offset = 0;
}
