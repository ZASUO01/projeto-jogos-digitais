# Shader de Grid Isométrico Neon - Documentação

## 📋 Visão Geral

Este shader implementa um grid isométrico com efeito neon estilo Tron/Cyberpunk, renderizado completamente no fragment shader usando técnicas avançadas de GPU.

## 🎯 Arquivos

- **AdvancedGrid.vert**: Vertex shader para quad full-screen
- **AdvancedGrid.frag**: Fragment shader com grid isométrico neon
- **AdvancedGridExample.cpp**: Exemplo de código C++ para uso

## 🔧 Técnicas Avançadas Implementadas

### 1. Projeção Isométrica no Fragment Shader
**Localização**: `AdvancedGrid.frag`, função `ToIsometricSpace()`

**O que é**: Transformação matemática que converte coordenadas de tela 2D para espaço isométrico diretamente no shader.

**Por que usar**: Permite renderizar o grid isométrico sem precisar gerar vértices isométricos em C++, tudo é calculado na GPU.

**Como funciona**:
- Usa transformação isométrica inversa com ângulo de 30 graus
- Fórmula: `x_iso = (x - y) * cos(30°)`, `y_iso = (x + y) * sin(30°)`
- Aplicada inversamente para converter coordenadas de tela para espaço isométrico

### 2. Distance Field Rendering
**Localização**: `AdvancedGrid.frag`, função `GridDistance()`

**O que é**: Técnica que usa a distância até as linhas do grid para determinar a intensidade visual.

**Por que usar**: Permite criar linhas suaves e efeitos de glow facilmente, sem precisar renderizar geometria complexa.

**Como funciona**:
- Calcula distância até as bordas da célula usando `fract()` para repetição
- Retorna a menor distância até qualquer linha (horizontal ou vertical)
- Essa distância é usada para calcular o efeito neon

### 3. Smoothstep para Glow Neon
**Localização**: `AdvancedGrid.frag`, função `CalculateGlow()`

**O que é**: Uso da função `smoothstep()` do GLSL para criar transições suaves que simulam brilho neon.

**Por que usar**: `smoothstep()` cria uma curva de interpolação suave (ease-in-out) perfeita para efeitos de glow.

**Como funciona**:
- Cria um glow principal na linha usando `smoothstep(LINE_WIDTH, LINE_WIDTH * 0.1, dist)`
- Adiciona um halo mais amplo com `smoothstep(LINE_WIDTH * GLOW_FALLOFF, 0.0, dist)`
- Combina ambos para criar o efeito neon característico

### 4. Fract para Repetição Infinita
**Localização**: `AdvancedGrid.frag`, função `GridDistance()`

**O que é**: Uso da função `fract()` para criar padrões repetidos infinitamente.

**Por que usar**: Permite que o grid se estenda por toda a tela sem calcular muitas células individualmente.

**Como funciona**:
- `fract(x)` retorna apenas a parte fracionária de `x` (ex: `fract(3.7) = 0.7`)
- Aplicado às coordenadas isométricas divididas por `GRID_SIZE`
- Cria um padrão repetido que se estende infinitamente

## 📦 Uniforms Necessários

### uResolution (vec2)
**Tipo**: `vec2` (width, height)  
**Por que existe**: O shader precisa saber o tamanho da tela para calcular coordenadas de fragmento corretamente usando `gl_FragCoord.xy / uResolution`.

**Como passar**:
```cpp
gridShader->SetVector2Uniform("uResolution", Vector2(screenWidth, screenHeight));
```

### uTime (float)
**Tipo**: `float` (tempo em segundos)  
**Por que existe**: Permite animações como pulsação, movimento de linhas, etc. O shader usa isso para criar efeitos dinâmicos.

**Como passar**:
```cpp
float currentTime = SDL_GetTicks() / 1000.0f; // Converte milissegundos para segundos
gridShader->SetFloatUniform("uTime", currentTime);
```

### uColor (vec3)
**Tipo**: `vec3` (RGB, valores de 0.0 a 1.0)  
**Por que existe**: Permite controlar a cor do grid neon. Você pode alterar para diferentes estilos visuais.

**Cores sugeridas**:
- **Azul Tron**: `(0.0, 0.7, 1.0)`
- **Ciano**: `(0.0, 1.0, 1.0)`
- **Rosa Cyberpunk**: `(1.0, 0.0, 1.0)`
- **Verde Matrix**: `(0.0, 1.0, 0.0)`
- **Laranja**: `(1.0, 0.5, 0.0)`

**Como passar**:
```cpp
Vector3 neonColor(0.0f, 0.7f, 1.0f); // Azul ciano estilo Tron
gridShader->SetVectorUniform("uColor", neonColor);
```

## 🚀 Como Usar

### Passo 1: Carregar o Shader
```cpp
Shader* gridShader = new Shader();
if (!gridShader->Load("Shaders/AdvancedGrid"))
{
    SDL_Log("Erro ao carregar shader AdvancedGrid");
    return;
}
```

### Passo 2: Criar Quad Full-Screen
```cpp
float vertices[] = {
    -1.0f, -1.0f,  // Canto inferior esquerdo
     1.0f, -1.0f,  // Canto inferior direito
     1.0f,  1.0f,  // Canto superior direito
    -1.0f,  1.0f   // Canto superior esquerdo
};

unsigned int indices[] = {
    0, 1, 2,  // Primeiro triângulo
    0, 2, 3   // Segundo triângulo
};

VertexArray* fullScreenQuad = new VertexArray(vertices, 8, indices, 6);
```

### Passo 3: No Loop de Renderização
```cpp
// Ativa o shader
gridShader->SetActive();

// Configura uniforms
gridShader->SetVector2Uniform("uResolution", Vector2(screenWidth, screenHeight));
gridShader->SetFloatUniform("uTime", SDL_GetTicks() / 1000.0f);
gridShader->SetVectorUniform("uColor", Vector3(0.0f, 0.7f, 1.0f));

// Habilita blending para efeito de glow
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// Desenha o quad
fullScreenQuad->SetActive();
glDrawElements(GL_TRIANGLES, fullScreenQuad->GetNumIndices(), GL_UNSIGNED_INT, nullptr);

glDisable(GL_BLEND);
```

## ⚙️ Parâmetros Ajustáveis

No fragment shader, você pode ajustar estas constantes para personalizar o visual:

```glsl
const float GRID_SIZE = 0.05;           // Tamanho das células (menor = mais células)
const float LINE_WIDTH = 0.002;         // Espessura das linhas
const float GLOW_INTENSITY = 2.5;       // Intensidade do brilho neon
const float GLOW_FALLOFF = 8.0;         // Quão rápido o glow decai
const float DEPTH_FADE = 0.3;           // Fator de fade por profundidade
```

**Dica**: Para tornar esses valores uniformes (controláveis em tempo real), substitua `const float` por `uniform float` e passe os valores do C++.

## 🎨 Características Visuais

- ✅ Grid isométrico com perspectiva 3D
- ✅ Linhas neon brilhantes com efeito glow
- ✅ Gradiente de profundidade (fade por distância)
- ✅ Fundo escuro com leve gradiente vertical
- ✅ Pulsação sutil baseada no tempo
- ✅ Sem necessidade de texturas externas
- ✅ Renderização completamente na GPU (muito eficiente)

## 🔍 Compatibilidade

- **OpenGL**: 3.3+ (Core Profile)
- **GLSL**: 330
- **Pipeline**: VAO/VBO padrão
- **Plataforma**: Windows, Linux, macOS (com SDL2 + OpenGL)

## 📝 Notas Técnicas

1. **Performance**: O shader é muito eficiente pois todo o cálculo é feito na GPU. Mesmo com milhões de pixels, a performance é excelente.

2. **Blending**: O blending é necessário para o efeito de glow funcionar corretamente. Certifique-se de habilitá-lo antes de desenhar.

3. **Coordenadas**: O vertex shader passa coordenadas normalizadas (0.0 a 1.0) para o fragment shader através de `fragCoord`.

4. **Isométrico vs Ortogonal**: A projeção isométrica cria a ilusão de profundidade 3D em uma superfície 2D, característica de jogos como Age of Empires, SimCity, etc.

## 🐛 Troubleshooting

**Grid não aparece**:
- Verifique se o shader foi compilado corretamente (veja logs do SDL)
- Certifique-se de que `uResolution` está sendo passado corretamente
- Verifique se o blending está habilitado

**Grid muito grande/pequeno**:
- Ajuste a constante `GRID_SIZE` no fragment shader
- Valores menores = mais células (grid mais denso)
- Valores maiores = menos células (grid mais espaçado)

**Cores não aparecem**:
- Verifique se `uColor` está sendo passado com valores entre 0.0 e 1.0
- Certifique-se de que o blending está configurado corretamente

**Performance ruim**:
- Este shader é muito eficiente, mas se houver problemas, tente reduzir `GLOW_INTENSITY` ou `GLOW_FALLOFF`

