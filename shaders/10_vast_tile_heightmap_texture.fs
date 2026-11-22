#version 330 core

in vec2 v_uv;

layout(location = 0) out float outHeight;

uniform vec2  u_tile_offset;  /* where the tile starts                   */
uniform float u_tile_size;    /* size of a tile (e.g 256.0f world units) */
uniform float u_texture_size; /* size of the output height map           */

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
        
        n   = 1.0 - abs(n * 2.0 - 1.0);
        f  += w * n * n;
        w  *= -0.42;
        pos = rotate2D * pos;
    }

    float ff = Noise(pos * 0.002);
    ff *= ff;
    f  += ff * ff * 280.0 - 5.0;
    f  += max(f - 15.0, 0.0) * 0.35;

    return f;
}

void main()
{
    vec2 worldPos =
        ((gl_FragCoord.xy - vec2(0.5)) /
        (u_texture_size - 1.0) - vec2(0.5) +
        u_tile_offset) * u_tile_size;

    outHeight = Terrain(worldPos);

    // TODO(nickscha): make terrain more streched
    // float stretch = 2.0; // tiwce as streched horizontally
    // float hScale  = 1.5; // scale height
    // outHeight = Terrain(worldPos / stretch) * hScale;
}
