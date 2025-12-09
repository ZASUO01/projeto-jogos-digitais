#version 330

// Vertex shader para efeito CRT (TV de tubo)
// Desenha um quad full-screen para aplicar o efeito de pós-processamento

layout (location = 0) in vec2 inPosition;

out vec2 fragCoord;
out vec2 screenCoord;

void main()
{
    // Posição do vértice em clip space (-1 a 1)
    gl_Position = vec4(inPosition, 0.0, 1.0);
    
    // Coordenadas normalizadas (0.0 a 1.0) para o fragment shader
    fragCoord = (inPosition + 1.0) * 0.5;
    
    // Coordenadas de tela para cálculos de distorção
    screenCoord = inPosition;
}

