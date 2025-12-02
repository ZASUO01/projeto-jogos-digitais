#version 330

out vec4 outColor;
uniform vec3 uColor;
uniform float uTime;
uniform vec2 uResolution;

void main()
{
    vec2 uv = gl_FragCoord.xy / uResolution;

    // Efeito de pulsação suave
    float pulse = sin(uTime * 2.0) * 0.1 + 0.9;

    // Gradiente para glow
    float alpha = 0.3 * pulse;

    // Adicionar um leve brilho azulado
    vec3 glowColor = uColor * 1.2 + vec3(0.1, 0.2, 0.4) * pulse;

    outColor = vec4(glowColor, alpha);
}