#version 330 core

layout (location = 0) in vec2  a_uv;
layout (location = 1) in float a_is_skirt;
layout (location = 2) in vec2  a_instance_offset;
layout (location = 3) in vec3  a_instance_data; /* texture_array_layer, is_dirty, transition_factor */

uniform mat4  u_vp;
uniform float u_tile_size;
uniform float u_water_level;
uniform float iTime;

out vec2       v_uv;
out vec3       v_worldPos;
out float      v_waveHeight;
out float      v_is_skirt;
flat out float v_texture_array_layer;

float calcSurfaceWave(vec2 xz, float t)
{
    vec2 dir1 = normalize(vec2(0.82, 0.57));
    vec2 dir2 = normalize(vec2(-0.46, 0.89));

    float w1 = sin(dot(xz, dir1) * 0.045 - t * 1.35);
    float w2 = sin(dot(xz, dir2) * 0.085 - t * 1.90 + 1.7);

    return (w1 * 0.65 + w2 * 0.35) * 0.35;
}

void main()
{
    v_uv = a_uv;

    vec2 local_pos = (v_uv - vec2(0.5)) * u_tile_size;
    vec2 world_pos = local_pos + (a_instance_offset * u_tile_size);

    float waveH = 0.0;
    if (a_is_skirt < 0.5)
    {
        float edgeDist = min(min(a_uv.x, 1.0 - a_uv.x), min(a_uv.y, 1.0 - a_uv.y));
        float edgeFade = smoothstep(0.0, 0.05, edgeDist);
        waveH    = calcSurfaceWave(world_pos, iTime) * edgeFade;
    }

    v_texture_array_layer = a_instance_data.x;
    v_waveHeight          = waveH;
    v_is_skirt            = a_is_skirt;
    v_worldPos            = vec3(world_pos.x, u_water_level + waveH, world_pos.y);
    gl_Position           = u_vp * vec4(v_worldPos, 1.0);
}