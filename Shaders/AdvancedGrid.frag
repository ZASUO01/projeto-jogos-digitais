#version 330

// Fragment shader para grid isométrico neon estilo Tron/Cyberpunk
// TÉCNICAS AVANÇADAS UTILIZADAS:
// 1. Projeção isométrica no fragment shader (transformação de coordenadas)
// 2. Distance field rendering (uso de distância para criar linhas)
// 3. Smoothstep para efeitos de glow suave
// 4. Fract para repetição de padrões

in vec2 fragCoord;
out vec4 outColor;

// Uniforms
uniform vec2 uResolution;      // Resolução da tela (width, height)
uniform float uTime;            // Tempo para animações (em segundos)
uniform vec3 uColor;            // Cor base do grid (pode ser azul, ciano, rosa, etc.)

// Parâmetros do grid (podem ser uniforms se quiser controlar em tempo real)
const float GRID_SIZE = 0.05;           // Tamanho das células do grid (menor = mais células)
const float LINE_WIDTH = 0.002;         // Espessura das linhas (em espaço isométrico)
const float GLOW_INTENSITY = 2.5;       // Intensidade do brilho neon
const float GLOW_FALLOFF = 8.0;         // Quão rápido o glow decai
const float DEPTH_FADE = 0.3;           // Fator de fade por profundidade

// Função para converter coordenadas de tela para espaço isométrico
// TÉCNICA: Projeção isométrica 2D (ângulo de ~30 graus)
vec2 ToIsometricSpace(vec2 screenCoord)
{
    // Constantes para projeção isométrica
    // Ângulo típico de 30 graus: cos(30°) ≈ 0.866, sin(30°) = 0.5
    const float ISO_ANGLE = 0.523598776; // 30 graus em radianos
    const float COS_ISO = 0.866025404;   // cos(30°)
    const float SIN_ISO = 0.5;           // sin(30°)
    
    // Ajusta coordenadas para centro em (0,0) e escala adequada
    vec2 centered = (screenCoord - 0.5) * 2.0;
    
    // Aplica transformação isométrica inversa
    // Em isométrico: x_iso = (x - y) * cos(30°), y_iso = (x + y) * sin(30°)
    // Inverso: x = (x_iso/cos + y_iso/sin) / 2, y = (y_iso/sin - x_iso/cos) / 2
    float isoX = (centered.x / COS_ISO + centered.y / SIN_ISO) * 0.5;
    float isoY = (centered.y / SIN_ISO - centered.x / COS_ISO) * 0.5;
    
    return vec2(isoX, isoY);
}

// Função para calcular distância até a linha de grid mais próxima
// TÉCNICA: Distance field rendering usando fract() para repetição
float GridDistance(vec2 isoCoord)
{
    // Usa fract() para criar padrão repetido infinito
    vec2 gridCoord = fract(isoCoord / GRID_SIZE);
    
    // Calcula distância até as bordas da célula (0.0 e 1.0)
    // Isso cria linhas nas bordas de cada célula
    vec2 distToEdge = min(gridCoord, 1.0 - gridCoord);
    
    // Retorna a menor distância até qualquer linha (horizontal ou vertical)
    // Multiplica por GRID_SIZE para manter escala correta
    return min(distToEdge.x, distToEdge.y) * GRID_SIZE;
}

// Função para calcular intensidade do glow baseada na distância
// TÉCNICA: Smoothstep para transições suaves estilo neon
float CalculateGlow(float dist, float pulseFactor)
{
    // A pulsação afeta a espessura aparente das linhas
    // pulseFactor está entre 0.7 e 1.3, então normalizamos para 0.0-1.0
    // e aplicamos uma variação de 20% na espessura
    float pulseNormalized = (pulseFactor - 0.7) / 0.6; // 0.0 a 1.0
    float dynamicLineWidth = LINE_WIDTH * (0.8 + pulseNormalized * 0.2);
    
    // Smoothstep cria uma curva suave de 0 a 1
    // Quanto mais próximo da linha, maior a intensidade
    float lineGlow = smoothstep(dynamicLineWidth, dynamicLineWidth * 0.1, dist);
    
    // Adiciona um halo mais amplo ao redor da linha
    // O halo também pulsa junto de forma proporcional
    float haloGlow = smoothstep(dynamicLineWidth * GLOW_FALLOFF, 0.0, dist) * (0.3 * pulseFactor);
    
    return lineGlow + haloGlow;
}

// Função para calcular fade por profundidade (efeito de perspectiva)
float CalculateDepthFade(vec2 isoCoord)
{
    // Coordenada Y isométrica representa "profundidade"
    // Quanto maior Y, mais "longe" está na perspectiva isométrica
    float depth = isoCoord.y * 0.5 + 0.5; // Normaliza para 0-1
    
    // Fade suave baseado na profundidade
    return 1.0 - smoothstep(0.0, 1.0, depth) * DEPTH_FADE;
}

void main()
{
    // Converte coordenadas de fragmento para espaço de tela normalizado
    vec2 screenCoord = gl_FragCoord.xy / uResolution;
    
    // TÉCNICA: Projeção isométrica no fragment shader
    // Converte coordenadas de tela para espaço isométrico
    vec2 isoCoord = ToIsometricSpace(screenCoord);
    
    // TÉCNICA: Distance field rendering
    // Calcula distância até a linha de grid mais próxima
    float dist = GridDistance(isoCoord);
    
    // Efeito de pulsação baseado no tempo
    // Usa função seno simples e direta: sin() retorna -1 a 1
    // Normaliza para 0.0 a 1.0: (sin() + 1.0) * 0.5
    // Escala para range 0.7 a 1.3: 0.7 + normalized * 0.6
    float pulse = 0.7 + (sin(uTime * 2.0) + 1.0) * 0.3;
    
    // TÉCNICA: Smoothstep para glow neon
    // Calcula intensidade do brilho baseada na distância
    // Passa a pulsação para afetar a espessura das linhas
    float glow = CalculateGlow(dist, pulse);
    
    // Calcula fade por profundidade para efeito de perspectiva
    float depthFade = CalculateDepthFade(isoCoord);
    
    // Combina todos os efeitos - a pulsação afeta o glow
    float finalGlow = glow * depthFade * pulse;
    
    // Cor neon com variação de intensidade
    // A cor base (uColor) é multiplicada pela intensidade do glow
    // A pulsação já está incluída no finalGlow, então aplicamos uma leve intensificação
    vec3 neonColor = uColor * finalGlow * GLOW_INTENSITY;
    
    // Adiciona um leve gradiente vertical ao fundo (quase preto)
    float bgGradient = 0.05 + screenCoord.y * 0.05;
    vec3 backgroundColor = vec3(bgGradient * 0.1);
    
    // Combina cor neon com fundo
    vec3 finalColor = backgroundColor + neonColor;
    
    // Alpha baseado na intensidade do glow (para efeito de transparência)
    float alpha = min(1.0, finalGlow * 0.8 + 0.2);
    
    outColor = vec4(finalColor, alpha);
}
