#version 330

out vec4 outColor;
uniform vec3 uColor;
uniform float uTime;
uniform vec2 uResolution;
uniform vec2 uMousePos;

void main()
{
    vec2 uv = gl_FragCoord.xy / uResolution;
    vec2 mouseUV = uMousePos / uResolution;

    // Efeito de distorção baseada no tempo
    float distortion = sin(uv.y * 20.0 + uTime * 3.0) * 0.002;
    uv.x += distortion;

    // Glow com gradiente radial
    float distFromCenter = length(uv - vec2(0.5));
    float glow = (1.0 - distFromCenter) * 0.5;

    // Efeito de interação com mouse
    float mouseDist = length(uv - mouseUV);
    float mouseGlow = max(0.0, 0.3 - mouseDist * 0.5);

    // Combinação de cores com efeitos
    vec3 finalColor = uColor * (1.0 + glow + mouseGlow);
    float alpha = 0.4 + glow * 0.3 + mouseGlow;

    // Adicionar cor dinâmica baseada no tempo
    finalColor += vec3(sin(uTime), cos(uTime * 0.7), sin(uTime * 1.3)) * 0.1;

    outColor = vec4(finalColor, alpha);
}