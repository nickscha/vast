#version 330 core

layout (location = 0) in vec2  a_uv;
layout (location = 1) in float a_is_skirt;
layout (location = 2) in vec2  a_instance_offset;
layout (location = 3) in vec2  a_instance_data; /* texture_array_layer, is_dirty */

uniform mat4           u_vp;
uniform float          u_tile_size;
uniform float          u_grid_res;
uniform sampler2DArray u_heightmap;
uniform float          u_visualization_mode;

out float      v_is_dirty_pass;
out vec2       v_uv;
out vec3       v_worldPos;
flat out float v_texture_array_layer;

void main()
{
    v_uv = a_uv;
    
    vec2 local_pos = (v_uv - vec2(0.5)) * u_tile_size;
    vec2 world_pos = local_pos + (a_instance_offset * u_tile_size);

    int v_x = int(round(v_uv.x * (u_grid_res - 1.0)));
    int v_z = int(round(v_uv.y * (u_grid_res - 1.0)));

    float height = texelFetch(u_heightmap, ivec3(v_x, v_z, int(a_instance_data.x)), 0).r;

    if (u_visualization_mode > 3.5 && u_visualization_mode < 4.5) {
        height = 0.0;
    }

    if (a_is_skirt > 0.5f) {
        height -= 50.0f;
    }
    
    v_is_dirty_pass       = a_instance_data.y;
    v_texture_array_layer = a_instance_data.x;
    v_worldPos            = vec3(world_pos.x, height, world_pos.y);
    gl_Position           = u_vp * vec4(v_worldPos, 1.0);
}
