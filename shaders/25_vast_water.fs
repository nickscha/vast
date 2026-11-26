#version 330 core

in vec2       v_uv;
in vec3       v_worldPos;
in float      v_waveHeight;
in float      v_is_skirt;
flat in float v_texture_array_layer;

uniform sampler2DArray u_heightmap;
uniform float          u_water_level;
uniform float          u_texture_size;
uniform float          iTime;
uniform vec3           sunDir;
uniform vec3           iCamera;

out vec4 FragColor;

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise2D(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm2(vec2 p)
{
    mat2 rot = mat2(0.80, 0.60, -0.60, 0.80);
    float v = 0.0;
    float a = 0.55;
    for (int i = 0; i < 2; i++)
    {
        v += a * noise2D(p);
        p = rot * p * 2.07;
        a *= 0.45;
    }
    return v;
}

float sampleSmoothHeight(sampler2DArray tex, vec2 uv, float layer, float texSize)
{
    float maxIdx    = max(texSize - 1.0, 1.0);
    vec2 pixelCoord = clamp(uv, 0.0, 1.0) * maxIdx;
    ivec2 baseCoord = ivec2(floor(pixelCoord));
    vec2 f          = fract(pixelCoord);

    int maxCoord    = int(texSize) - 1;
    baseCoord       = clamp(baseCoord, ivec2(0), ivec2(maxCoord));
    ivec2 nextCoord = min(baseCoord + ivec2(1), ivec2(maxCoord));

    float h00 = texelFetch(tex, ivec3(baseCoord.x, baseCoord.y, int(layer)), 0).r;
    float h10 = texelFetch(tex, ivec3(nextCoord.x, baseCoord.y, int(layer)), 0).r;
    float h01 = texelFetch(tex, ivec3(baseCoord.x, nextCoord.y, int(layer)), 0).r;
    float h11 = texelFetch(tex, ivec3(nextCoord.x, nextCoord.y, int(layer)), 0).r;

    return mix(mix(h00, h10, f.x), mix(h01, h11, f.x), f.y);
}

vec3 getFogColor(vec3 rd, vec3 sDir)
{
    float h = max(rd.y, 0.0);

    float dayAmount    = clamp(sDir.y * 0.5 + 0.5, 0.0, 1.0);
    float nightAmount  = 1.0 - dayAmount;
    float sunsetAmount = exp(-abs(sDir.y) * 7.0);

    vec3 dayZenith     = vec3(0.08, 0.30, 0.80);
    vec3 dayHorizon    = vec3(0.70, 0.85, 1.00);
    vec3 sunsetZenith  = vec3(0.70, 0.25, 0.35);
    vec3 sunsetHorizon = vec3(1.00, 0.45, 0.10);
    vec3 nightZenith   = vec3(0.005, 0.010, 0.020);
    vec3 nightHorizon  = vec3(0.020, 0.030, 0.050);

    vec3 zenith  = mix(nightZenith, dayZenith, dayAmount);
    vec3 horizon = mix(nightHorizon, dayHorizon, dayAmount);

    zenith  = mix(zenith, sunsetZenith, sunsetAmount);
    horizon = mix(horizon, sunsetHorizon, sunsetAmount);

    vec3 sky = mix(horizon, zenith, pow(h, 0.35));

    float sunAmount = max(dot(rd, sDir), 0.0);
    float sunGlow   = pow(sunAmount, 32.0);
    vec3 sunColor   = mix(vec3(1.0, 0.95, 0.85), vec3(1.0, 0.45, 0.20), sunsetAmount);
    sky += sunColor * sunGlow * 0.6;

    vec3 moonDir     = -sDir;
    float moonAmount = max(dot(rd, moonDir), 0.0);
    float moonGlow   = pow(moonAmount, 20.0);
    sky += vec3(0.8, 0.85, 1.0) * moonGlow * 0.08 * nightAmount;

    float horizonGlow = pow(1.0 - h, 8.0);
    sky += sunsetHorizon * horizonGlow * sunsetAmount * 0.5;

    return sky;
}

float cloudHash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float cloudValueNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(cloudHash(i + vec2(0.0, 0.0)), cloudHash(i + vec2(1.0, 0.0)), u.x),
        mix(cloudHash(i + vec2(0.0, 1.0)), cloudHash(i + vec2(1.0, 1.0)), u.x),
        u.y
    );
}

float getCloudShadow(vec3 pos, vec3 sun_dir, float time)
{
    if (sun_dir.y < 0.15) return 1.0;

    float safeSunY    = max(sun_dir.y, 0.05);
    float cloudHeight = 1200.0;
    float t           = (cloudHeight - pos.y) / safeSunY;

    if (t < 0.0) return 1.0;

    vec3 cloudPos    = pos + sun_dir * t;
    vec2 windDir2D   = normalize(vec2(1.0, 1.0));
    float windSpeed  = 0.04;
    vec2 windOffset  = windDir2D * time * windSpeed;
    vec2 uv          = cloudPos.xz * 0.0012 + windOffset;

    float density = 0.0;
    float a       = 0.5;
    mat2 rot      = mat2(0.80, 0.60, -0.60, 0.80);

    for (int i = 0; i < 3; i++)
    {
        density += a * cloudValueNoise(uv);
        uv = rot * uv * 2.02;
        a *= 0.5;
    }

    density = smoothstep(0.38, 0.75, density);

    float shadowMask  = smoothstep(0.0, 0.4, density);
    float horizonFade = smoothstep(0.0, 0.15, sun_dir.y);
    shadowMask *= horizonFade;

    return mix(1.0, 0.5, shadowMask);
}

float waterWaveField(vec2 xz, float time, float detailFade)
{
    vec2 swellCoord = xz * 0.035 - vec2(time * 0.45, time * 0.30);
    float swell = sin(swellCoord.x + cos(swellCoord.y * 1.15)) * 0.5 + 0.5;

    if (detailFade <= 0.001)
    {
        return swell * 0.4;
    }

    vec2 c1 = xz * 0.075 - vec2(time * 0.85, time * 0.60);
    vec2 c2 = xz * 0.190 + vec2(time * 0.65, -time * 0.95);

    float n1 = fbm2(c1);
    float n2 = fbm2(c2);

    return (swell * 0.35 + (n1 * 0.6 + n2 * 0.4) * 0.65) * detailFade;
}

vec3 getAnimatedWaterNormal(vec2 xz, float time, float dist)
{
    float detailFade = 1.0 - smoothstep(25.0, 260.0, dist);
    float normalFade = 1.0 - smoothstep(80.0, 600.0, dist);
    float eps        = max(0.08, dist * 0.0025);

    float hC = waterWaveField(xz, time, detailFade);
    float hX = waterWaveField(xz + vec2(eps, 0.0), time, detailFade);
    float hZ = waterWaveField(xz + vec2(0.0, eps), time, detailFade);

    float strength = mix(0.45, 0.12, smoothstep(10.0, 180.0, dist)) * normalFade;
    return normalize(vec3((hC - hX) * strength / eps, 1.0, (hC - hZ) * strength / eps));
}

void main()
{
    if (v_is_skirt > 0.001)
    {
        discard;
    }

    float terrainHeight = sampleSmoothHeight(u_heightmap, v_uv, v_texture_array_layer, u_texture_size);
    float waterDepth    = v_worldPos.y - terrainHeight;
    float shoreWarp     = (noise2D(v_worldPos.xz * 0.14 + vec2(iTime * 0.18, -iTime * 0.14)) - 0.5) * 0.85;
    float perturbedDep  = waterDepth + shoreWarp;

    if (perturbedDep <= 0.0)
    {
        discard;
    }

    vec3 cameraToFrag = v_worldPos - iCamera;
    float dist        = length(cameraToFrag);
    vec3 rayDir       = cameraToFrag / max(dist, 0.0001);
    vec3 viewDir      = -rayDir;
    vec3 sDir         = normalize(sunDir);

    vec3 normal = getAnimatedWaterNormal(v_worldPos.xz, iTime, dist);

    vec3 deepWater  = vec3(0.015, 0.048, 0.068);
    vec3 shallowCol = vec3(0.032, 0.105, 0.095);
    vec3 foamColor  = vec3(0.78,  0.84,  0.86);

    float depthFactor = 1.0 - exp(-max(waterDepth, 0.0) * 0.22);
    vec3 mate = mix(shallowCol, deepWater, depthFactor);

    // Foam
    float animPhase  = iTime * 1.6;
    float rippleRing = 0.5 + 0.5 * sin(perturbedDep * 7.5 - animPhase);
    float fNoise     = fbm2(v_worldPos.xz * 0.22 + vec2(iTime * 0.25, iTime * 0.17));
    float shoreBand  = (1.0 - smoothstep(0.05, 1.6, perturbedDep)) * (0.45 + 0.55 * fNoise);
    float lapBand    = (1.0 - smoothstep(0.4, 2.6, perturbedDep)) * smoothstep(0.55, 0.92, rippleRing * (0.5 + 0.7 * fNoise));
    float crestFoam  = smoothstep(0.18, 0.42, v_waveHeight) * smoothstep(0.58, 0.85, fNoise) * smoothstep(0.5, 2.5, waterDepth) * 0.35;
    float foamFade   = 1.0 - smoothstep(120.0, 550.0, dist);
    float foamMask   = clamp((shoreBand + lapBand * 0.75 + crestFoam) * foamFade, 0.0, 1.0);

    mate = mix(mate, foamColor, foamMask);

    // Ligthning
    float dayAmount    = clamp(sDir.y * 0.5 + 0.5, 0.0, 1.0);
    float sunsetAmount = exp(-abs(sDir.y) * 7.0);
    float wrap         = 0.25;
    float sunDif       = max(dot(normal, sDir) + wrap, 0.0) / (1.0 + wrap);
    sunDif             = pow(sunDif, 1.4);
    float moonDif      = max(dot(normal, -sDir), 0.0);
    vec3 sunLightCol   = mix(vec3(1.30, 1.15, 0.90), vec3(1.00, 0.45, 0.20), sunsetAmount);
    vec3 moonLightCol  = vec3(0.08, 0.12, 0.18);
    vec3 zenith        = mix(vec3(0.005, 0.010, 0.020), vec3(0.08, 0.30, 0.80), dayAmount);
    zenith             = mix(zenith, vec3(0.70, 0.25, 0.35), sunsetAmount);

    // Cloud Shadow
    float cloudShadow = getCloudShadow(v_worldPos, sDir, iTime);

    vec3 light = vec3(0.0);
    light += sunDif * sunLightCol * cloudShadow;
    light += moonDif * moonLightCol;
    light += zenith * 0.55;
    light += mix(vec3(0.03, 0.035, 0.04), zenith, max(normal.y, 0.0)) * 0.45;

    vec3 waterLit = mate * light;

    vec3 reflDir = reflect(-viewDir, normal);
    reflDir.y    = max(reflDir.y, 0.02);
    vec3 skyRefl = getFogColor(reflDir, sDir) * mix(0.65, 1.0, cloudShadow);

    // Schlick Fresnel
    float cosTheta = clamp(dot(normal, viewDir), 0.0, 1.0);
    float fresnel  = 0.02 + 0.98 * pow(1.0 - cosTheta, 5.0);
    fresnel        = clamp(fresnel * 0.85 * (1.0 - foamMask * 0.75), 0.0, 0.92);

    // Specular highlighting
    vec3 halfVec   = normalize(sDir + viewDir);
    float specPow  = pow(max(dot(normal, halfVec), 0.0), 160.0);
    float specGain = mix(0.20, 0.12, foamMask);
    vec3 specLight = specPow * specGain * sunLightCol * cloudShadow;

    vec3 col = mix(waterLit, skyRefl, fresnel) + specLight;

    // Fog calculation
    vec3 fogCol     = getFogColor(rayDir, sDir);
    float distFog   = 1.0 - exp(-dist * 0.00015);
    float heightFog = exp(-max(v_worldPos.y, 0.0) * 0.015);
    float totalFog  = clamp(distFog * heightFog, 0.0, 1.0);

    col = mix(col, fogCol, totalFog);

    // Far water less saturated
    float distant   = smoothstep(100.0, 500.0, dist);
    float luminance = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(col, vec3(luminance), distant * 0.15);

    // Tone mapping & Gamma collection
    col = max(col, vec3(0.0));
    col = col / (col + vec3(1.0));
    col = pow(col, vec3(0.4545));

    float alpha = smoothstep(0.0, 0.8, perturbedDep) * 0.78 + foamMask * 0.22;
    alpha = clamp(alpha, 0.0, 1.0);

    FragColor = vec4(col, alpha);
}