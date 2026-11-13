#version 330 core

in float      v_is_dirty_pass;
in vec2       v_uv;
in vec3       v_worldPos;
in float      v_transition_factor;
flat in float v_texture_array_layer;

uniform float          u_visualization_mode;
uniform sampler2DArray u_normalmap;
uniform float          u_grid_res;

uniform vec3  iResolution;
uniform vec3  iMouse;
uniform float iTime;
uniform vec3  sunDir;
uniform vec3  iCamera;
uniform vec3  iViewDir;

out vec4 FragColor;

void main()
{
    vec3 normal = texture(u_normalmap, vec3(v_uv, v_texture_array_layer)).rgb * 2.0 - 1.0;

    vec3 col = normal;

    // Tone mapping
    col = max(col, vec3(0.0));  
    col = col / (col + vec3(1.0));

    // Gamma Correction
    col = pow(col, vec3(0.4545));

    FragColor = vec4(col, 1.0);
}
