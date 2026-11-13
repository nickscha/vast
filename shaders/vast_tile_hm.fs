#version 330 core

in vec2 v_uv;

layout(location = 0) out float outHeight;

uniform vec2 u_tile_offset;
uniform float u_tile_size;
uniform float u_texture_size;

#define HASHSCALE1 0.1031

const vec2 add = vec2(1.0, 0.0);
const mat2 rotate2D = mat2(1.3623, 1.7531, -1.7131, 1.4623);

float Hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float Noise(vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(Hash12(p),          Hash12(p + add.xy), f.x),
        mix(Hash12(p + add.yx), Hash12(p + add.xx), f.x),
        f.y
    );
}

float Terrain(vec2 p)
{
    vec2 pos = p * 0.01;
    float w = Noise(pos * 0.25) * 0.75 + 0.15;
    float f = 0.0;

    w = 66.0 * w * w;

    for(int i = 0; i < 6; i++)
    {
        float n = Noise(pos);
        
        n = 1.0 - abs(n * 2.0 - 1.0);
        n = n * n;

        f += w * n;
        w *= -0.42;
        pos = rotate2D * pos;
    }

    float ff = Noise(pos * 0.002);
    f += pow(abs(ff), 4.0) * 280.0 - 5.0;

    if (f > 15.0) {
        f += (f - 15.0) * 0.35;
    }

    return f;
}

void main()
{
    vec2 grid_pos = gl_FragCoord.xy - vec2(0.5);
    vec2 local_pos = (grid_pos / (u_texture_size - 1.0) - vec2(0.5)) * u_tile_size;
    vec2 worldPos = local_pos + (u_tile_offset * u_tile_size);

    outHeight = Terrain(worldPos);
}
