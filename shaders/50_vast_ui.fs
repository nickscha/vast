#version 330 core

in vec2      v_uv;
in vec4      v_color;
flat in uint v_isSolid;

uniform sampler2D tex_font;

out vec4 FragColor;

void main()
{
    if (v_isSolid == 1u)
    {
        FragColor = v_color;
        return;
    }

    vec4 texel       = texture(tex_font, v_uv);
    float dist       = texel.a;
    float aa         = max(fwidth(dist) * 0.75, 0.001);
    float sdfAlpha   = smoothstep(0.5 - aa, 0.5 + aa, dist);
    float texelsPp   = max(fwidth(v_uv.x) * 1024.0, fwidth(v_uv.y) * 1024.0);
    float smallBlend = smoothstep(2.0, 3.5, texelsPp);
    float alpha      = mix(sdfAlpha, texel.r, smallBlend);

    FragColor = vec4(v_color.rgb, alpha * v_color.a);
}
