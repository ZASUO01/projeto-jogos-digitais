#version 330

// Fragment shader para efeito CRT (TV de tubo) retrô
// Simula distorções, scanlines, vignetting e outros efeitos de TVs antigas

in vec2 fragCoord;
in vec2 screenCoord;
out vec4 outColor;

uniform sampler2D uSceneTexture;
uniform vec2 uResolution;
uniform float uTime;

// Parâmetros do efeito CRT
const float CURVATURE = 0.25;         // Intensidade da curvatura da tela (aumentado para distorção mais visível)
const float SCANLINE_INTENSITY = 0.35; // Intensidade das scanlines
const float SCANLINE_COUNT = 480.0;   // Número de scanlines (simula resolução antiga)
const float VIGNETTE_INTENSITY = 0.6; // Intensidade do vignetting
const float CHROMATIC_ABERRATION = 0.008; // Intensidade da aberração cromática (aumentado)
const float BRIGHTNESS = 1.1;         // Brilho geral
const float CONTRAST = 1.2;           // Contraste
const float NOISE_INTENSITY = 0.10;   // Intensidade do ruído/static
const float BORDER_SHADOW_SIZE = 0.10; // Tamanho da sombra da moldura da TV (mais curta)
const float BORDER_SHADOW_INTENSITY = 1.0; // Intensidade da sombra da moldura (mais forte)

// Função para gerar ruído pseudo-aleatório
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Função para gerar ruído suave
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// Aplica curvatura da tela (distorção nas bordas)
vec2 ApplyCurvature(vec2 coord) {
    // Centraliza as coordenadas
    vec2 centered = coord - 0.5;
    
    // Calcula a distância do centro
    float dist = length(centered);
    
    // Aplica curvatura mais intensa (quadrática e cúbica para distorção mais pronunciada)
    float curvature = dist * dist * CURVATURE + dist * dist * dist * CURVATURE * 0.5;
    vec2 curved = centered * (1.0 + curvature);
    
    // Retorna coordenadas centralizadas
    return curved + 0.5;
}

// Aplica aberração cromática (separação de cores RGB)
vec3 ApplyChromaticAberration(sampler2D tex, vec2 coord) {
    // Aumenta a aberração nas bordas (mais distorção)
    vec2 centered = coord - 0.5;
    float dist = length(centered);
    float aberrationFactor = CHROMATIC_ABERRATION * (1.0 + dist * 2.0); // Mais intenso nas bordas
    
    vec2 offset = centered * aberrationFactor;
    
    float r = texture(tex, coord + offset).r;
    float g = texture(tex, coord).g;
    float b = texture(tex, coord - offset).b;
    
    return vec3(r, g, b);
}

// Calcula scanlines (linhas horizontais)
float CalculateScanlines(vec2 coord) {
    float scanline = sin(coord.y * SCANLINE_COUNT * 3.14159) * 0.5 + 0.5;
    return 1.0 - scanline * SCANLINE_INTENSITY;
}

// Calcula vignetting (escurecimento nas bordas)
float CalculateVignette(vec2 coord) {
    vec2 centered = coord - 0.5;
    float dist = length(centered);
    return 1.0 - smoothstep(0.3, 1.0, dist) * VIGNETTE_INTENSITY;
}

// Calcula sombra da moldura da TV (simula a borda física da TV de tubo)
float CalculateBorderShadow(vec2 coord) {
    // Calcula a distância até as bordas
    float distToLeft = coord.x;
    float distToRight = 1.0 - coord.x;
    float distToTop = coord.y;
    float distToBottom = 1.0 - coord.y;
    
    // Encontra a distância mínima até qualquer borda
    float minDist = min(min(distToLeft, distToRight), min(distToTop, distToBottom));
    
    // Cria uma sombra que é mais intensa nas bordas (minDist = 0)
    // e desaparece gradualmente em direção ao centro
    // smoothstep(0.0, BORDER_SHADOW_SIZE, minDist) retorna:
    // - 0.0 quando minDist = 0 (borda - queremos sombra máxima)
    // - 1.0 quando minDist >= BORDER_SHADOW_SIZE (centro - sem sombra)
    float shadowFactor = smoothstep(0.0, BORDER_SHADOW_SIZE, minDist);
    
    // Invertemos: nas bordas (shadowFactor = 0) queremos sombra máxima
    // No centro (shadowFactor = 1) queremos sem sombra
    // shadow = 1.0 significa sem sombra, valores menores significam mais escuro
    float shadow = mix(1.0 - BORDER_SHADOW_INTENSITY, 1.0, shadowFactor);
    
    return shadow;
}

void main()
{
    // Aplica curvatura da tela
    vec2 curvedCoord = ApplyCurvature(fragCoord);
    
    // Verifica se está dentro dos limites (evita artefatos nas bordas)
    if (curvedCoord.x < 0.0 || curvedCoord.x > 1.0 || 
        curvedCoord.y < 0.0 || curvedCoord.y > 1.0) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    
    // Aplica aberração cromática
    vec3 color = ApplyChromaticAberration(uSceneTexture, curvedCoord);
    
    // Aplica scanlines
    float scanlineFactor = CalculateScanlines(fragCoord);
    color *= scanlineFactor;
    
    // Aplica vignetting
    float vignette = CalculateVignette(fragCoord);
    color *= vignette;
    
    // Aplica sombra da moldura da TV (efeito de borda física)
    float borderShadow = CalculateBorderShadow(fragCoord);
    color *= borderShadow;
    
    // Adiciona ruído/static sutil
    float staticNoise = noise(fragCoord * uResolution + uTime * 10.0) * NOISE_INTENSITY;
    color += staticNoise;
    
    // Ajusta brilho e contraste
    color = (color - 0.5) * CONTRAST + 0.5;
    color *= BRIGHTNESS;
    
    // Aplica leve desfoque nas bordas (simula foco imperfeito)
    float edgeBlur = smoothstep(0.0, 0.1, min(min(fragCoord.x, 1.0 - fragCoord.x), 
                                               min(fragCoord.y, 1.0 - fragCoord.y)));
    vec3 blurredColor = ApplyChromaticAberration(uSceneTexture, curvedCoord + vec2(0.001, 0.001));
    color = mix(blurredColor, color, edgeBlur);
    
    // Garante que as cores estão no range válido
    color = clamp(color, 0.0, 1.0);
    
    outColor = vec4(color, 1.0);
}

