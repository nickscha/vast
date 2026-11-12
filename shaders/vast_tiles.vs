#version 330 core

layout (location = 0) in vec2  a_uv;
layout (location = 1) in float a_is_skirt;
layout (location = 2) in vec2  a_instance_offset;
layout (location = 3) in vec3  a_instance_data; /* texture_array_layer, is_dirty, transition_factor */

uniform mat4           u_vp;
uniform float          u_tile_size;
uniform float          u_grid_res;
uniform sampler2DArray u_heightmap;

out float      v_is_dirty_pass;
out vec2       v_uv;
out vec3       v_worldPos;
out float      v_transition_factor;
flat out float v_texture_array_layer;

void main()
{
    v_uv = a_uv;
    
    vec2 local_pos = (v_uv - vec2(0.5)) * u_tile_size;
    vec2 world_pos = local_pos + (a_instance_offset * u_tile_size);

    int v_x = int(round(v_uv.x * (u_grid_res - 1.0)));
    int v_z = int(round(v_uv.y * (u_grid_res - 1.0)));

    float height = texelFetch(u_heightmap, ivec3(v_x, v_z, int(a_instance_data.x)), 0).r;

    // LOD High to Low: Slowly transition from coarse mesh to finer version
    if (a_instance_data.z < 1.0) {
        int coarse_x1 = (v_x / 2) * 2;
        int coarse_x2 = min(coarse_x1 + 2, int(u_grid_res - 1));
        
        int coarse_z1 = (v_z / 2) * 2;
        int coarse_z2 = min(coarse_z1 + 2, int(u_grid_res - 1));

        float h00 = texelFetch(u_heightmap, ivec3(coarse_x1, coarse_z1, int(a_instance_data.x)), 0).r;
        float h10 = texelFetch(u_heightmap, ivec3(coarse_x2, coarse_z1, int(a_instance_data.x)), 0).r;
        float h01 = texelFetch(u_heightmap, ivec3(coarse_x1, coarse_z2, int(a_instance_data.x)), 0).r;
        float h11 = texelFetch(u_heightmap, ivec3(coarse_x2, coarse_z2, int(a_instance_data.x)), 0).r;

        float fx = float(v_x - coarse_x1) / 2.0;
        float fz = float(v_z - coarse_z1) / 2.0;
        float height_coarse = mix(mix(h00, h10, fx), mix(h01, h11, fx), fz);

        height = mix(height_coarse, height, a_instance_data.z);
    }

    if (a_is_skirt > 0.5f) {
        height -= 50.0f;
    }
    
    v_is_dirty_pass       = a_instance_data.y;
    v_texture_array_layer = a_instance_data.x;
    v_transition_factor   = a_instance_data.z;
    v_worldPos            = vec3(world_pos.x, height, world_pos.y);
    gl_Position           = u_vp * vec4(v_worldPos, 1.0);
}
