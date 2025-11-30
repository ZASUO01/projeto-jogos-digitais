#version 330

// Fragment shader para grid isométrico neon estilo Tron/Cyberpunk com projeção isométrica, distance field rendering e efeitos de glow

in vec2 fragCoord;
out vec4 outColor;

uniform vec2 uResolution;
uniform float uTime;
uniform vec3 uColor;

const float GRID_SIZE = 0.05;
const float LINE_WIDTH = 0.002;
const float GLOW_INTENSITY = 2.5;
const float GLOW_FALLOFF = 8.0;
const float DEPTH_FADE = 0.3;

vec2 ToIsometricSpace(vec2 screenCoord)
{
    const float COS_ISO = 0.866025404;
    const float SIN_ISO = 0.5;
    
    vec2 centered = (screenCoord - 0.5) * 2.0;
    float isoX = (centered.x / COS_ISO + centered.y / SIN_ISO) * 0.5;
    float isoY = (centered.y / SIN_ISO - centered.x / COS_ISO) * 0.5;
    
    return vec2(isoX, isoY);
}

float GridDistance(vec2 isoCoord)
{
    vec2 gridCoord = fract(isoCoord / GRID_SIZE);
    vec2 distToEdge = min(gridCoord, 1.0 - gridCoord);
    return min(distToEdge.x, distToEdge.y) * GRID_SIZE;
}

float CalculateGlow(float dist, float pulseFactor)
{
    float pulseNormalized = (pulseFactor - 0.7) / 0.6;
    float dynamicLineWidth = LINE_WIDTH * (0.8 + pulseNormalized * 0.2);
    
    float lineGlow = smoothstep(dynamicLineWidth, dynamicLineWidth * 0.1, dist);
    float haloGlow = smoothstep(dynamicLineWidth * GLOW_FALLOFF, 0.0, dist) * (0.3 * pulseFactor);
    
    return lineGlow + haloGlow;
}

float CalculateDepthFade(vec2 isoCoord)
{
    float depth = isoCoord.y * 0.5 + 0.5;
    return 1.0 - smoothstep(0.0, 1.0, depth) * DEPTH_FADE;
}

void main()
{
    vec2 screenCoord = gl_FragCoord.xy / uResolution;
    vec2 isoCoord = ToIsometricSpace(screenCoord);
    float dist = GridDistance(isoCoord);
    
    float pulse = 0.7 + (sin(uTime * 2.0) + 1.0) * 0.3;
    float glow = CalculateGlow(dist, pulse);
    float depthFade = CalculateDepthFade(isoCoord);
    float finalGlow = glow * depthFade * pulse;
    
    vec3 neonColor = uColor * finalGlow * GLOW_INTENSITY;
    float bgGradient = 0.05 + screenCoord.y * 0.05;
    vec3 backgroundColor = vec3(bgGradient * 0.1);
    vec3 finalColor = backgroundColor + neonColor;
    
    float alpha = min(1.0, finalGlow * 0.8 + 0.2);
    outColor = vec4(finalColor, alpha);
}
