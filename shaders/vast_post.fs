#version 330 core

in vec2 vUV;

uniform sampler2D u_scene_tex;

out vec4 FragColor;

void main()
{
    vec3 col = texture(u_scene_tex, vUV).rgb;
    FragColor = vec4(col, 1.0);
}
