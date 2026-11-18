#version 330 core

layout(location = 0) in vec4 a_rect;
layout(location = 1) in vec4 a_color;
layout(location = 2) in uint a_glyph_index;

uniform vec3 iResolution;

out vec2      v_uv;
out vec4      v_color;
flat out uint v_isSolid;

void main()
{
    vec2 corner   = vec2(gl_VertexID & 1, (gl_VertexID >> 1) & 1);
    vec2 pixelPos = a_rect.xy + corner * a_rect.zw;
    vec2 ndc      = (pixelPos / iResolution.xy) * 2.0 - 1.0;

    ndc.y = -ndc.y;

     // rectangle solid fill
    if (a_glyph_index == 424242u)
    {
        v_isSolid = 1u;
        v_uv = vec2(0.0);
    }
    else
    {
        float col = float(a_glyph_index & 15u);
        float row = float(15u - (a_glyph_index >> 4u));

        v_isSolid = 0u;
        v_uv = (vec2(col, row) + vec2(corner.x, 1.0 - corner.y)) / 16.0;
    }

    v_color = a_color;
    gl_Position = vec4(ndc, 0.0, 1.0);
}
