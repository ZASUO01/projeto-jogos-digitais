# Lion Casters

Jogo 2D de duelo espacial multiplayer para dois jogadores. O código está em C++17, usa SDL2 + OpenGL/GLEW para renderização e áudio, e opcionalmente FFmpeg para reprodução de vídeos da abertura.

## Youtube video

Assista ao gameplay/overview no YouTube: [Lion Casters vídeo](https://www.youtube.com/watch?v=0_NzYstbGKg).

## Requisitos para rodar/compilar
- CMake 3.26+ e um compilador C++17
- SDL2
- SDL2_image
- SDL2_ttf
- SDL2_mixer
- GLEW
- OpenGL (3.3+)
- FFmpeg (avcodec, avformat, avutil, swscale) – opcional, necessário para reproduzir os vídeos de abertura; sem ele o jogo roda, mas os vídeos do menu inicial não serão exibidos.

## Como compilar e executar (Linux)
1) Instale as dependências (exemplo em Debian/Ubuntu):  
   `sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libglew-dev libavcodec-dev libavformat-dev libavutil-dev libswscale-dev`
2) Compile:  
   ```
   mkdir -p build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build .
   ```
3) Execute a partir da raiz do projeto (para resolver caminhos de assets):  
   `./build/line-casters`

## Estrutura rápida
- `Source/` – motor do jogo, UI (menus, HUD, telas de conexão e fim de jogo), lógica de combate, partículas, shaders e reprodução de vídeo/áudio.
- `Client/` e `Network/` – infraestrutura de cliente/rede utilizada pelas telas de conexão.
- `Assets/` – fontes e sons usados em runtime.
- `Opening/` – vídeos e áudios da sequência de abertura.
- `Shaders/` – shaders GLSL usados no renderizador.

## Controles
### Jogador 1 (Nave Azul/Ciano)
- **Movimentação**: W (cima), A (esquerda), S (baixo), D (direita)
- **Tiro**: **SPACE** (Barra de Espaço)

### Jogador 2 (Nave Vermelha)
- **Movimentação**: ↑ (cima), ← (esquerda), ↓ (baixo), → (direita)
- **Tiro**: **ENTER** ou **CTRL DIREITO**

### Controles Gerais
- **ESC**: Sair do jogo (durante o gameplay)
- **ENTER**: Pular vídeo introdutório / Confirmar seleção no menu

## Descrição do gameplay
Cada nave começa com 4 vidas. Ao tomar dano, o jogador fica invencível por ~2s e existe um cooldown de tiro de ~0.2s. O cenário usa grade neon e partículas; a UI mostra vidas, menus, telas de abertura e fim de jogo. Com FFmpeg instalado, os vídeos de `Opening/` são reproduzidos na sequência inicial.

## O que observar em testes
- Colisão e dano (vidas decrementam corretamente e invencibilidade de 2s ativa).
- Cooldown de tiro e rotação das naves em 8 direções.
- Reprodução dos vídeos e transições de telas (abertura → menu → gameplay → fim).
- Áudio de fundo, tiros e efeitos.
- Performance estável (~60 FPS) sem travamentos.

## Screenshots
![Menu e HUD](ScreenShots/WhatsApp%20Image%202025-12-09%20at%2021.02.29.jpeg)
![Gameplay 1](ScreenShots/WhatsApp%20Image%202025-12-09%20at%2021.02.29(1).jpeg)
![Gameplay 2](ScreenShots/WhatsApp%20Image%202025-12-09%20at%2021.07.46.jpeg)
![Gameplay 3](ScreenShots/WhatsApp%20Image%202025-12-09%20at%2021.07.46(1).jpeg)
![Gameplay 4](ScreenShots/WhatsApp%20Image%202025-12-09%20at%2021.07.46(2).jpeg)
![Gameplay 5](ScreenShots/WhatsApp%20Image%202025-12-09%20at%2021.07.46(3).jpeg)

## Créditos
- **Chrystian Martins** - [Shaders, efeitos e mecanicas básicas]
- **Isabela Saenz Cardoso** - [Sistema de Telas e menu inicial]
- **Pedro Souza** - [Sistema de conexao e ajustes no codigo]
- **Fabio Marra** - [Sistema de audio e definições]

