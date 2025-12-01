# Lion Casters

## Screenshots

_Adicione aqui capturas de tela mostrando diferentes momentos do jogo, como:_
- Tela de abertura com vídeo introdutório
- Menu principal
- Gameplay com as duas naves em combate
- Sistema de vidas e interface do jogo

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

## Descrição

**Lion Casters** é um jogo de combate espacial multiplayer local onde dois jogadores controlam naves espaciais em uma arena 2D. O objetivo principal é eliminar a nave adversária através de combate direto, utilizando projéteis laser e manobras estratégicas.

O jogo apresenta uma experiência imersiva com vídeo introdutório, trilha sonora dinâmica e um sistema de combate baseado em vidas. Cada nave possui 3 vidas e, ao receber dano, entra em um período de invencibilidade temporária de 2 segundos, adicionando uma camada estratégica ao combate. Os jogadores podem se mover em 8 direções (cardeais e diagonais) usando os controles: **Jogador 1** utiliza **W, A, S, D** para movimentação e **SPACE** para atirar, enquanto o **Jogador 2** usa as **setas direcionais** para movimento e **ENTER** ou **CTRL DIREITO** para disparar. O sistema de tiro possui um cooldown de 0.2 segundos, criando momentos de tensão e decisões táticas importantes durante as batalhas.

As condições de vitória e derrota são simples: o jogador vence quando consegue reduzir todas as vidas do oponente a zero, enquanto perde quando sua própria nave é destruída. O jogo oferece feedback visual através de um sistema de partículas, indicadores de colisão e uma interface que mostra as vidas restantes de cada nave, facilitando a leitura do estado do combate em tempo real.

## Funcionalidades a serem testadas

Durante o playtesting, preste atenção especial às seguintes funcionalidades:

### Sistema de Combate
- **Colisão de lasers**: Verifique se os lasers detectam corretamente as colisões com as naves inimigas
- **Sistema de vidas**: Confirme que o sistema de vidas diminui corretamente ao receber dano
- **Invencibilidade temporária**: Observe se o período de invencibilidade de 2 segundos funciona corretamente após tomar dano
- **Cooldown de tiro**: Teste se o sistema de cooldown impede tiros excessivamente rápidos

### Controles e Movimentação
- **Controles do Jogador 1 (WASD)**: Verifique se os controles W, A, S, D funcionam corretamente para movimentação e direção
- **Tiro do Jogador 1**: Teste se a tecla **SPACE (Barra de Espaço)** dispara os lasers corretamente
- **Controles do Jogador 2 (Setas)**: Confirme se as setas direcionais respondem adequadamente
- **Tiro do Jogador 2**: Verifique se as teclas **ENTER** ou **CTRL DIREITO** disparam os lasers corretamente
- **Movimentação em 8 direções**: Teste se a nave se move corretamente nas direções cardeais e diagonais
- **Rotação da nave**: Observe se a nave rotaciona adequadamente conforme a direção do movimento
- **Cooldown de tiro**: Confirme se há um intervalo adequado entre os tiros (0.2 segundos)

### Sistema de Cenas
- **Tela de abertura**: Verifique se o vídeo introdutório carrega e reproduz corretamente
- **Transições de vídeo**: Teste as transições entre begin.mp4, abertura.mp4 e entrance_loop.mp4
- **Menu principal**: Confirme se o menu aparece corretamente após o vídeo de abertura
- **Início do jogo**: Verifique se a transição do menu para o gameplay funciona sem problemas

### Áudio
- **Sincronização de áudio**: Teste se os áudios (begin.wav, abertura.wav, loop.wav) tocam nos momentos corretos
- **Áudio de fundo**: Confirme se o loop de áudio toca como som de fundo durante o gameplay
- **Som de tiro**: Verifique se o som Shoot.wav toca quando as naves atiram

### Interface e Visual
- **Indicador de vidas**: Observe se as vidas são exibidas corretamente na tela
- **Sistema de partículas**: Teste se as partículas aparecem nos momentos apropriados
- **Grid de fundo**: Verifique se o grid isométrico neon renderiza corretamente
- **Colisores visuais**: Confirme se os círculos de colisão são visíveis (se habilitado o modo debug)

### Performance
- **Frame rate**: Observe se o jogo mantém ~60 FPS durante o gameplay
- **Sincronização**: Teste se não há travamentos ou lag durante combates intensos
- **Gerenciamento de memória**: Verifique se não há vazamentos de memória após múltiplas partidas

## Créditos

### Integrantes do Grupo

- **Chrystian Martins** - [Atribuições específicas]
- **Isabela Saenz Cardoso** - [Atribuições específicas]
- **Pedro Souza** - [Atribuições específicas]
- **Fabio Marra** - [Atribuições específicas]

### Atribuições de Tarefas

_Descreva aqui a divisão de trabalho entre os membros do grupo, por exemplo:_

- **Sistema de Cenas e Gerenciamento de Estado**: [Nome do integrante]
- **Sistema de Combate e Colisões**: [Nome do integrante]
- **Sistema de Vídeo e Áudio**: [Nome do integrante]
- **Interface e UI**: [Nome do integrante]
- **Sistema de Renderização**: [Nome do integrante]
- **Game Design e Balanceamento**: [Nome do integrante]

---

**Desenvolvido como projeto acadêmico - 2025**

