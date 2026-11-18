#version 330 core

in vec2      v_uv;
in vec4      v_color;
flat in uint v_isSolid;

uniform sampler2D tex_font;
uniform vec3      iResolution;
uniform float     iTime;

out vec4 FragColor;

void main()
{
    if (v_isSolid == 1u)
    {
        FragColor = v_color;
        return;
    }
    
    vec4 texel          = textureGrad(tex_font, v_uv, dFdx(v_uv), dFdy(v_uv));
    float dist          = texel.a;
    float aa            = fwidth(dist) * 0.55;
    float sdfAlpha      = 1.0 - smoothstep(0.5 - aa, 0.5 + aa, dist);
    float bitmapAlpha   = texel.r;
    vec2 texelsPerPixel = fwidth(v_uv) * vec2(textureSize(tex_font, 0));
    float minification  = max(texelsPerPixel.x, texelsPerPixel.y);
    float smallBlend    = smoothstep(2.0, 3.5, minification);
    float alpha         = mix(sdfAlpha, bitmapAlpha, smallBlend);
    float finalAlpha    = alpha * v_color.a;
    
    if (finalAlpha <= 0.001)
    {
        discard;
    }

    FragColor = vec4(v_color.rgb, finalAlpha);
}
