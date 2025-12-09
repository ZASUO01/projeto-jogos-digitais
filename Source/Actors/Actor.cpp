#include "Actor.h"
#include "../Game.h"
#include "../Components/Component.h"
#include <algorithm>

// Constrói um ator e o adiciona ao jogo
Actor::Actor(Game* game)
        : mState(ActorState::Active)
        , mPosition(Vector2::Zero)
        , mScale(Vector2(1.0f, 1.0f))
        , mRotation(0.0f)
        , mGame(game)
{
    mGame->AddActor(this);
}

// Remove o ator do jogo e deleta todos os seus componentes
Actor::~Actor()
{
    mGame->RemoveActor(this);
    unsigned int size = mComponents.size();
    for (unsigned int i = 0; i < size; i++) {
        delete mComponents[i];
    }
    mComponents.clear();
}

// Atualiza o ator e seus componentes se estiver ativo
void Actor::Update(float deltaTime)
{
    if (mState == ActorState::Active)
    {
        OnUpdate(deltaTime);
        unsigned int size = mComponents.size();
        for (unsigned int i = 0; i < size; i++) {
            mComponents[i]->Update(deltaTime);
        }
    }
}

// Método virtual para atualização específica do ator (pode ser sobrescrito)
void Actor::OnUpdate(float deltaTime){}

// Processa entrada do teclado para o ator e seus componentes se estiver ativo
void Actor::ProcessInput(const Uint8* keyState)
{
   if (mState == ActorState::Active)
    {
        OnProcessInput(keyState);
        unsigned int size = mComponents.size();
       for (unsigned int i = 0; i < size; i++) {
           mComponents[i]->ProcessInput(keyState);
       }
    }
}

// Método virtual para processamento de entrada específico do ator (pode ser sobrescrito)
void Actor::OnProcessInput(const Uint8* keyState){}

// Adiciona um componente ao ator e ordena por ordem de atualização
void Actor::AddComponent(Component* c)
{
    mComponents.emplace_back(c);
    std::sort(
        mComponents.begin(),
        mComponents.end(),
        [](const Component* a, const Component* b) {
            return a->GetUpdateOrder() < b->GetUpdateOrder();
        });
}

// Calcula e retorna a matriz de transformação do modelo (escala * rotação * translação)
Matrix4 Actor::GetModelMatrix() const
{
    Matrix4 scaleMat = Matrix4::CreateScale(mScale.x, mScale.y, 1.0f);
    Matrix4 rotMat   = Matrix4::CreateRotationZ(mRotation);
    Matrix4 transMat = Matrix4::CreateTranslation(Vector3(mPosition.x, mPosition.y, 0.0f));
    return scaleMat * rotMat * transMat;
}
