#include "Floor.h"
#include "../Game.h"
#include "../Components/GridDrawComponent.h"
#include "../Math.h"

// Constrói o ator do chão (grid é renderizado pelo shader AdvancedGrid no Renderer)
Floor::Floor(class Game* game)
    : Actor(game)
{
}

Floor::~Floor()
{
}

