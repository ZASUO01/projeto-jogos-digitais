#version 330

// Vertex shader para grid isométrico neon
// Este shader desenha um quad full-screen que cobre toda a tela

layout (location = 0) in vec2 inPosition;

// Uniforms para transformação (opcionais, mas mantidos para compatibilidade)
uniform mat4 uWorldTransform;
uniform mat4 uOrthoProj;

// Coordenadas de tela normalizadas (0.0 a 1.0) - passadas para o fragment shader
out vec2 fragCoord;

void main()
{
    // Passa as coordenadas do vértice diretamente (já estão em espaço de tela)
    // O quad será desenhado de -1 a 1 em ambos os eixos
    gl_Position = vec4(inPosition, 0.0, 1.0);
    
    // Converte coordenadas de clip space (-1 a 1) para coordenadas de tela (0 a 1)
    // Isso será usado no fragment shader para calcular o grid
    fragCoord = (inPosition + 1.0) * 0.5;
}

