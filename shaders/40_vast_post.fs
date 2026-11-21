#version 330 core

in vec2 vUV;

uniform sampler2D u_scene_tex;
uniform vec3      iResolution;
uniform float     iTime;
uniform vec2      u_sunScreenPos;

out vec4 FragColor;

vec4 fxaa(sampler2D tex, vec2 uv, vec2 texelSz) 
{
    const float span_max       = 8.0;  
    const float reduce_min     = 1.0 / 128.0;
    const float reduce_mul     = 1.0 / 32.0;
    const vec3  luma           = vec3(0.299, 0.587, 0.114);

    vec3 rgbCC = texture(tex, uv).rgb;
    vec3 rgb00 = texture(tex, uv + vec2(-0.5, -0.5) * texelSz).rgb;
    vec3 rgb10 = texture(tex, uv + vec2(+0.5, -0.5) * texelSz).rgb;
    vec3 rgb01 = texture(tex, uv + vec2(-0.5, +0.5) * texelSz).rgb;
    vec3 rgb11 = texture(tex, uv + vec2(+0.5, +0.5) * texelSz).rgb;

    float lumaCC = dot(rgbCC, luma);
    float luma00 = dot(rgb00, luma);
    float luma10 = dot(rgb10, luma);
    float luma01 = dot(rgb01, luma);
    float luma11 = dot(rgb11, luma);

    float lumaMin = min(lumaCC, min(min(luma00, luma10), min(luma01, luma11)));
    float lumaMax = max(lumaCC, max(max(luma00, luma10), max(luma01, luma11)));
    
    float lumaRange = lumaMax - lumaMin;
    float lumaThreshold = max(0.05, lumaMax * 0.12);
    
    if (lumaRange < lumaThreshold) {
        return vec4(rgbCC, 1.0);
    }

    vec2 dir = vec2((luma01 + luma11) - (luma00 + luma10), (luma00 + luma01) - (luma10 + luma11));
    float dirReduce = max((luma00 + luma10 + luma01 + luma11) * reduce_mul, reduce_min);

    float rcpDir = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDir, -span_max, span_max) * texelSz.xy;

    vec4 A = 0.5 * (texture(tex, uv - dir * (1.0 / 6.0)) + texture(tex, uv + dir * (1.0 / 6.0)));
    vec4 B = A * 0.5 + 0.25 * (texture(tex, uv - dir * 0.5) + texture(tex, uv + dir * 0.5));

    float lumaB = dot(B.rgb, luma);

    return ((lumaB < lumaMin) || (lumaB > lumaMax)) ? A : B;
}

float grain(vec2 uv, float time) 
{
    vec2 seed = uv * iResolution.xy + fract(time) * 1000.0;
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453) - 0.5;
}

void main()
{   
    // FXAA
    vec2 texelSize = 1.0 / iResolution.xy;
    vec3 col = fxaa(u_scene_tex, vUV, texelSize).rgb;

    // Vignette
    vec2 vigUV = vUV * (1.0 - vUV.yx);
    float vig = vigUV.x * vigUV.y * 15.0;
    vig = pow(clamp(vig, 0.0, 1.0), 0.25);
    col *= mix(1.0, vig, 0.25);          

    // Grain
    col += grain(vUV, iTime) * 0.025;

    // Cinematic Letterbox
    float targetAspect = 2.39; // 21:9
    float currentAspect = iResolution.x / iResolution.y;
    float maxBarHeight = max(0.0, (1.0 - (currentAspect / targetAspect)) * 0.5);
    float t = clamp((iTime - 5.0) / 3.0, 0.0, 1.0);
    float ease = t * t * (3.0 - 2.0 * t);
    float barHeight = maxBarHeight * (1.0 - ease);

    if (vUV.y < barHeight || vUV.y > 1.0 - barHeight) {
        col = vec3(0.0);
    }

    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
  
    //vec3 col = texture(u_scene_tex, vUV).rgb;
    //FragColor = vec4(col, 1.0);
}
