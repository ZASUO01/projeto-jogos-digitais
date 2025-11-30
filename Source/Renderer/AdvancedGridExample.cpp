//
// Exemplo de código C++ para usar o shader AdvancedGrid
// Este arquivo demonstra como compilar, carregar e usar o shader de grid isométrico neon
//

#include "Shader.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "../Math.h"
#include <GL/glew.h>
#include <SDL.h>

// ============================================================================
// EXEMPLO 1: Criar um quad full-screen para desenhar o grid
// ============================================================================

VertexArray* CreateFullScreenQuad()
{
    // Vértices de um quad que cobre toda a tela em espaço normalizado (-1 a 1)
    // Formato: x, y (2 componentes por vértice)
    float vertices[] = {
        -1.0f, -1.0f,  // Canto inferior esquerdo
         1.0f, -1.0f,  // Canto inferior direito
         1.0f,  1.0f,  // Canto superior direito
        -1.0f,  1.0f   // Canto superior esquerdo
    };
    
    // Índices para desenhar dois triângulos formando um quad
    unsigned int indices[] = {
        0, 1, 2,  // Primeiro triângulo
        0, 2, 3   // Segundo triângulo
    };
    
    // Cria e retorna o VertexArray
    // Parâmetros: vértices, número de floats (8 = 4 vértices * 2 componentes),
    //             índices, número de índices (6)
    return new VertexArray(vertices, 8, indices, 6);
}

// ============================================================================
// EXEMPLO 2: Função completa para desenhar o grid isométrico neon
// ============================================================================

void DrawAdvancedGrid(Renderer* renderer, float screenWidth, float screenHeight, float time)
{
    // 1. Carrega o shader (se ainda não foi carregado)
    // Nota: Em um projeto real, você carregaria isso uma vez na inicialização
    static Shader* gridShader = nullptr;
    static VertexArray* fullScreenQuad = nullptr;
    static bool initialized = false;
    
    if (!initialized)
    {
        // Cria e carrega o shader
        gridShader = new Shader();
        if (!gridShader->Load("Shaders/AdvancedGrid"))
        {
            SDL_Log("Erro ao carregar shader AdvancedGrid");
            return;
        }
        
        // Cria o quad full-screen
        fullScreenQuad = CreateFullScreenQuad();
        
        initialized = true;
    }
    
    // 2. Ativa o shader
    gridShader->SetActive();
    
    // 3. Configura os uniforms necessários
    
    // uResolution: Resolução da tela (width, height)
    // Por que existe: O shader precisa saber o tamanho da tela para calcular
    //                 coordenadas de fragmento corretamente (gl_FragCoord.xy / uResolution)
    gridShader->SetVector2Uniform("uResolution", Vector2(screenWidth, screenHeight));
    
    // uTime: Tempo em segundos para animações
    // Por que existe: Permite animações como pulsação, movimento, etc.
    //                 O shader usa isso para criar efeitos dinâmicos
    gridShader->SetFloatUniform("uTime", time);
    
    // uColor: Cor base do grid neon (RGB)
    // Por que existe: Permite controlar a cor do grid (azul, ciano, rosa, etc.)
    //                 Valores sugeridos:
    //                 - Azul Tron: (0.0, 0.7, 1.0)
    //                 - Ciano: (0.0, 1.0, 1.0)
    //                 - Rosa Cyberpunk: (1.0, 0.0, 1.0)
    //                 - Verde Matrix: (0.0, 1.0, 0.0)
    Vector3 neonColor(0.0f, 0.7f, 1.0f); // Azul ciano estilo Tron
    gridShader->SetVectorUniform("uColor", neonColor);
    
    // 4. Habilita blending para efeito de glow suave
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 5. Desenha o quad full-screen
    // O fragment shader calculará o grid isométrico neon para cada pixel
    fullScreenQuad->SetActive();
    glDrawElements(GL_TRIANGLES, fullScreenQuad->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
    
    // 6. Desabilita blending (opcional, dependendo do resto do código)
    glDisable(GL_BLEND);
}

// ============================================================================
// EXEMPLO 3: Integração em um loop de jogo típico
// ============================================================================

/*
void Game::UpdateGame(float deltaTime)
{
    // ... lógica do jogo ...
    
    // No loop de renderização:
    renderer->Clear();
    
    // Desenha o grid isométrico neon como fundo
    float currentTime = SDL_GetTicks() / 1000.0f; // Converte para segundos
    DrawAdvancedGrid(renderer, mScreenWidth, mScreenHeight, currentTime);
    
    // Desenha outros objetos por cima...
    
    renderer->Present();
}
*/

// ============================================================================
// EXPLICAÇÃO DAS TÉCNICAS AVANÇADAS UTILIZADAS
// ============================================================================

/*
1. PROJEÇÃO ISOMÉTRICA NO FRAGMENT SHADER
   Localização: AdvancedGrid.frag, função ToIsometricSpace()
   Como funciona: Converte coordenadas de tela (2D) para espaço isométrico
                  usando transformações matemáticas. Isso permite que o grid
                  seja renderizado diretamente no fragment shader sem precisar
                  gerar vértices isométricos em C++.

2. DISTANCE FIELD RENDERING
   Localização: AdvancedGrid.frag, função GridDistance()
   Como funciona: Usa a distância até as linhas do grid para determinar
                  a intensidade do efeito neon. Isso cria linhas suaves
                  e permite efeitos de glow facilmente.

3. SMOOTHSTEP PARA GLOW
   Localização: AdvancedGrid.frag, função CalculateGlow()
   Como funciona: A função smoothstep() cria uma curva suave de transição
                  que simula o efeito de brilho neon ao redor das linhas.
                  Quanto mais próximo da linha, maior a intensidade.

4. FRACT PARA REPETIÇÃO
   Localização: AdvancedGrid.frag, função GridDistance()
   Como funciona: A função fract() retorna a parte fracionária de um número,
                  criando um padrão repetido infinito. Isso permite que o
                  grid se estenda por toda a tela sem precisar calcular
                  muitas células individualmente.
*/

