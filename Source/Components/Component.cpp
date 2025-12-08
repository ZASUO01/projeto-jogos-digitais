#include "Component.h"
#include "../Actors/Actor.h"

// Constrói um componente e o adiciona ao ator proprietário
Component::Component(Actor* owner, int updateOrder)
          :mOwner(owner)
          ,mUpdateOrder(updateOrder)
{
    mOwner->AddComponent(this);
}

Component::~Component()
{
}

// Atualização do componente (pode ser sobrescrito)
void Component::Update(float deltaTime)
{
}

// Processamento de entrada do componente (pode ser sobrescrito)
void Component::ProcessInput(const Uint8* keyState)
{
}

// Desenho de debug do componente (pode ser sobrescrito)
void Component::DebugDraw(class Renderer* renderer)
{
}

// Retorna o jogo através do ator proprietário
class Game* Component::GetGame() const
{
    return mOwner->GetGame();
}


