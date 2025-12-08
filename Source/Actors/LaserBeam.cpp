#include "LaserBeam.h"
#include "../Game.h"
#include "../Components/LaserBeamComponent.h"
#include "Ship.h"
#include "../Renderer/AudioPlayer.h"

// Constrói um raio laser na posição inicial com rotação e cor especificadas
LaserBeam::LaserBeam(class Game* game, const Vector2& startPos, float rotation, Vector3 color, class Ship* ownerShip)
    : Actor(game)
    , mLaserComponent(nullptr)
    , mOwnerShip(ownerShip)
    , mShootSound(nullptr)
{
    SetPosition(startPos);
    SetRotation(rotation);
    
    float screenWidth = static_cast<float>(GetGame()->GetWindowWidth());
    float screenHeight = static_cast<float>(GetGame()->GetWindowHeight());
    
    mLaserComponent = new LaserBeamComponent(this, color, 0.5f);
    mLaserComponent->Activate(startPos, rotation, screenWidth, screenHeight, ownerShip);
    
    // Criar e tocar som de tiro
    mShootSound = new AudioPlayer();
    if (mShootSound->Load("../Assets/Sounds/Shoot.wav"))
    {
        mShootSound->SetVolume(64); // Volume reduzido (50% do máximo)
        mShootSound->Play(false);
    }
    else
    {
        delete mShootSound;
        mShootSound = nullptr;
    }
}

LaserBeam::~LaserBeam()
{
    if (mShootSound)
    {
        mShootSound->Stop();
        delete mShootSound;
        mShootSound = nullptr;
    }
}

// Destrói o laser quando seu componente não estiver mais ativo
void LaserBeam::OnUpdate(float deltaTime)
{
    if (mLaserComponent && !mLaserComponent->IsActive()) {
        SetState(ActorState::Destroy);
    }
}

