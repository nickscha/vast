#define VAST_APPLICATION
#include "vast.h"
#include "vast_state.h"

#include "vast_tile_renderer.h"
#include "vast_sky_renderer.h"
#include "vast_ui_renderer.h"

/* TODO(nickscha): Unclean !!! */
static vast_platform_api *api;

VAST_API f64 vast_profiler_time_ms(void)
{
    return api->time_ms_current();
}

VAST_API VAST_INLINE void vast_camera_update_movement(vast_platform_state *platform_state, vast_state *state)
{
    static u8 mouse_attached = 0;

    vast_camera *camera = &state->camera;
    f32 cam_speed = state->camera_speed * (f32)platform_state->timing.time_delta;
    vast_platform_input_keyboard *kb = &platform_state->input.keyboard;

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
    if (platform_state->input.mouse.keys_is_down[VAST_MOUSE_KEY_RIGHT] && !platform_state->input.mouse.keys_was_down[VAST_MOUSE_KEY_RIGHT])
    {
        mouse_attached = !mouse_attached;
    }

    if (mouse_attached)
    {
        f32 mouseSensitivity = 0.1f;
        camera->yaw += vast_minf((f32)platform_state->input.mouse.dx * mouseSensitivity, 89.0f);
        camera->pitch += vast_maxf((f32)platform_state->input.mouse.dy * mouseSensitivity, -89.0f);
        camera->pitch = vast_clampf(camera->pitch, -89.0f, 89.0f);

        if (platform_state->input.mouse.scroll != 0.0f)
        {
            camera->fov = vast_clampf(camera->fov - (platform_state->input.mouse.scroll * 2), 1.0f, 179.0f);
        }
    }

    vast_camera_update(camera);
}

VAST_API VAST_INLINE void vast_sun_update_movement(vast_platform_state *platform_state, vast_state *state)
{
    static u8 move_sun = 0;
    static u8 move_sun_loop = 0;
    static f32 move_val = 0.0f;

    vast_vec3 sun_dir;

    if (platform_state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_M] && !platform_state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_M])
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

        f32 t = (f32)platform_state->timing.time_elapsed * 0.5f;

        if (platform_state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_L] && !platform_state->input.keyboard.keys_was_down[VAST_KEYBOARD_KEY_L])
        {
            move_sun_loop = !move_sun_loop;
        }

        if (platform_state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_UP])
        {
            move_val += (f32)platform_state->timing.time_delta;
        }

        if (platform_state->input.keyboard.keys_is_down[VAST_KEYBOARD_KEY_DOWN])
        {
            move_val -= (f32)platform_state->timing.time_delta;

            if (move_val < 0.0f)
            {
                move_val = 0.0f;
            }
        }

        sun_dir = vast_vec3_init(0.0f, vast_sinf(move_val), -vast_cosf(move_val));

        if (move_sun_loop)
        {
            sun_dir = vast_vec3_init(0.0f, vast_sinf(t * 0.025f), -vast_cosf(t * 0.025f));
        }
    }

    state->sun_direction = sun_dir;
}

typedef struct shader_post_processing
{
    shader_header header;

    i32 loc_scene_texture;
    i32 loc_iResolution;
    i32 loc_iTime;
    i32 loc_sunScreenPos;

} shader_post_processing;

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

        scene_initialized = 1;
    }

    /* Update camera */
    vast_camera_update_movement(platform_state, state);

    /* Sun/Moon movement logic */
    vast_sun_update_movement(platform_state, state);

    state->rendering_draw_calls_terrain = 0;
    state->rendering_draw_calls_water = 0;
    state->rendering_draw_calls_sky = 0;
    state->rendering_draw_calls_post = 0;
    state->rendering_draw_calls_ui = 0;

    /******************************/
    /* Application Rendering      */
    /******************************/
    {
        static u8 post_process_initialized = 0;
        static u32 color_tex;
        static u32 depth_rbo;
        static shader_post_processing post_processing_shader = {0};
        static u32 post_processing_vao;
        static i32 last_width = 0;
        static i32 last_height = 0;

        static s8 *shader_code_vertex =
            "#version 330 core\n"
            "\n"
            "out vec2 vUV;\n"
            "\n"
            "const vec2 verts[3] = vec2[](\n"
            "    vec2(-1.0, -1.0),\n"
            "    vec2( 3.0, -1.0),\n"
            "    vec2(-1.0,  3.0)\n"
            ");\n"
            "\n"
            "void main()\n"
            "{\n"
            "    vec2 p = verts[gl_VertexID];\n"
            "    vUV = p * 0.5 + 0.5;\n"
            "    gl_Position = vec4(p, 1.0, 1.0);\n"
            "}";

        if (state->rendering_enabled_post)
        {

            vast_opengl_api *gl = &platform_state->graphics.opengl;

            if (!post_process_initialized)
            {
                gl->glGenFramebuffers(1, &state->post_framebuffer_object_id);
                gl->glBindFramebuffer(GL_FRAMEBUFFER, state->post_framebuffer_object_id);

                gl->glGenTextures(1, &color_tex);
                gl->glBindTexture(GL_TEXTURE_2D, color_tex);
                gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (i32)platform_state->window.width, (i32)platform_state->window.height, 0, GL_RGBA, GL_FLOAT, VAST_NULL);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);

                gl->glGenRenderbuffers(1, &depth_rbo);
                gl->glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
                gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, (i32)platform_state->window.width, (i32)platform_state->window.height);
                gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);

                gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

                /* Load shader */
                {
                    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&platform_state->api, &state->memory_region_transient, "shaders/40_vast_post.fs");

                    if (!shader_data_fragment)
                    {
                        return;
                    }

                    if (opengl_shader_load(&post_processing_shader.header, shader_code_vertex, (s8 *)shader_data_fragment, platform_state->api.io_print, gl))
                    {
                        post_processing_shader.loc_scene_texture = gl->glGetUniformLocation(post_processing_shader.header.program, "u_scene_tex");
                        post_processing_shader.loc_iResolution = gl->glGetUniformLocation(post_processing_shader.header.program, "iResolution");
                        post_processing_shader.loc_iTime = gl->glGetUniformLocation(post_processing_shader.header.program, "iTime");
                        post_processing_shader.loc_sunScreenPos = gl->glGetUniformLocation(post_processing_shader.header.program, "u_sunScreenPos");
                    }
                    else
                    {
                        platform_state->api.io_print("Cannot compile post processing shaders!\n");
                    }

                    gl->glGenVertexArrays(1, &post_processing_vao);
                }

                post_process_initialized = 1;
            }

            if (last_width != (i32)platform_state->window.width || last_height != (i32)platform_state->window.height)
            {

                last_width = (i32)platform_state->window.width;
                last_height = (i32)platform_state->window.height;

                gl->glBindTexture(GL_TEXTURE_2D, color_tex);
                gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (i32)platform_state->window.width, (i32)platform_state->window.height, 0, GL_RGBA, GL_FLOAT, VAST_NULL);

                gl->glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
                gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, (i32)platform_state->window.width, (i32)platform_state->window.height);
            }

            if (platform_state->timing.frame_count % 30 == 0)
            {
                static u8 hot_reload_initialized = 0;
                static u32 post_processing_shader_heightmap_last = 0;

                u32 post_processing_shader_heightmap_now = 0;

                if (!hot_reload_initialized)
                {
                    platform_state->api.io_file_mod_signature("shaders/40_vast_post.fs", &post_processing_shader_heightmap_last);

                    hot_reload_initialized = 1;
                }

                platform_state->api.io_file_mod_signature("shaders/40_vast_post.fs", &post_processing_shader_heightmap_now);

                if (post_processing_shader_heightmap_now != post_processing_shader_heightmap_last)
                {
                    u8 *shader_data_fragment = (u8 *)vast_io_file_read_to_memory(&platform_state->api, &state->memory_region_transient, "shaders/vast_post.fs");

                    if (!shader_data_fragment)
                    {
                        return;
                    }

                    if (opengl_shader_load(&post_processing_shader.header, shader_code_vertex, (s8 *)shader_data_fragment, platform_state->api.io_print, gl))
                    {
                        post_processing_shader.loc_scene_texture = gl->glGetUniformLocation(post_processing_shader.header.program, "u_scene_tex");
                        post_processing_shader.loc_iResolution = gl->glGetUniformLocation(post_processing_shader.header.program, "iResolution");
                        post_processing_shader.loc_iTime = gl->glGetUniformLocation(post_processing_shader.header.program, "iTime");
                        post_processing_shader.loc_sunScreenPos = gl->glGetUniformLocation(post_processing_shader.header.program, "u_sunScreenPos");
                    }
                    else
                    {
                        platform_state->api.io_print("Cannot compile post processing shaders!\n");
                    }
                }
            }

            gl->glBindFramebuffer(GL_FRAMEBUFFER, state->post_framebuffer_object_id);
            gl->glViewport(0, 0, (i32)platform_state->window.width, (i32)platform_state->window.height);
        }

        /* Main rendering */
        platform_state->graphics.opengl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vast_tile_render(platform_state, state);

        if (state->rendering_enabled_sky)
        {
            vast_sky_render(platform_state, state);
        }

        (void)vast_sb_f64;
        (void)vast_sb_s8_pad;

        /* Post Processing shader */
        if (state->rendering_enabled_post)
        {
            vast_opengl_api *gl = &platform_state->graphics.opengl;

            vast_mat4x4 projection = vast_mat4x4_perspective(VAST_DEG_TO_RAD(state->camera.fov), (f32)platform_state->window.width / (f32)platform_state->window.height, 0.1f, 8192.0f);
            vast_mat4x4 view = vast_mat4x4_look_at(state->camera.position, vast_vec3_add(state->camera.position, state->camera.forward), state->camera.up);
            vast_mat4x4 mvp = vast_mat4x4_mul(projection, view);

            vast_vec3 sun_world_pos = vast_vec3_add(state->camera.position, vast_vec3_mulf(state->sun_direction, 5000.0f));
            vast_vec4 sun_clip = vast_mat4x4_mul_vec4(mvp, vast_vec4_init(sun_world_pos.x, sun_world_pos.y, sun_world_pos.z, 1.0f));
            vast_vec2 sun_screen_pos = vast_vec2_init(0.5f, 0.5f);

            if (sun_clip.w > 0.0f)
            {
                sun_screen_pos.x = (sun_clip.x / sun_clip.w) * 0.5f + 0.5f;
                sun_screen_pos.y = (sun_clip.y / sun_clip.w) * 0.5f + 0.5f;
            }
            else
            {
                sun_screen_pos.x = -10.0f;
                sun_screen_pos.y = -10.0f;
            }

            gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
            gl->glViewport(0, 0, (i32)platform_state->window.width, (i32)platform_state->window.height);
            gl->glDisable(GL_DEPTH_TEST);

            gl->glUseProgram(post_processing_shader.header.program);
            gl->glUniform1i(post_processing_shader.loc_scene_texture, 0);
            gl->glUniform3f(post_processing_shader.loc_iResolution, (f32)platform_state->window.width, (f32)platform_state->window.height, 1.0f);
            gl->glUniform1f(post_processing_shader.loc_iTime, (f32)platform_state->timing.time_elapsed);
            gl->glUniform2f(post_processing_shader.loc_sunScreenPos, sun_screen_pos.x, sun_screen_pos.y);

            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, color_tex);
            gl->glBindVertexArray(post_processing_vao);
            gl->glDrawArrays(GL_TRIANGLES, 0, 3);

            state->rendering_draw_calls_post++;
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
