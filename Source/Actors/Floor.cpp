//
// Created for floor with grid
//

#include "Floor.h"
#include "../Game.h"
#include "../Components/GridDrawComponent.h"
#include "../Math.h"

Floor::Floor(class Game* game)
    : Actor(game)
{
    // Grid antigo removido - agora usamos o shader AdvancedGrid que é renderizado
    // diretamente no Renderer como fundo. Este Actor pode ser usado para outras
    // funcionalidades do chão no futuro, se necessário.
}

Floor::~Floor()
{
}

