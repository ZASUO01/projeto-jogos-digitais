//
// Created by Lucas N. Ferreira on 18/09/25.
//

#include "ParticleSystemComponent.h"
#include "../Game.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Random.h"

Particle::Particle(class Game* game, std::vector<Vector2> &vertices, SystemType type, Vector3 color, bool filled)
    : Actor(game)
    , mDrawComponent(nullptr)
    , mRigidBodyComponent(nullptr)
    , mCircleColliderComponent(nullptr)
    , mIsDead(true)
    , mLifeTime(1.0f)
    , mSystemType(type)
{
    mDrawComponent = new DrawComponent(this, vertices, 100, color, filled);
    mRigidBodyComponent = new RigidBodyComponent(this);
    mCircleColliderComponent = new CircleColliderComponent(this, 2);

    mState = ActorState::Paused;
    mDrawComponent->SetVisible(false);
}

void Particle::Kill()
{
    mIsDead = true;
    mState = ActorState::Paused;
    mDrawComponent->SetVisible(false);
    mRigidBodyComponent->SetVelocity(Vector2(0.0f, 0.0f));
}

void Particle::Awake(const Vector2 &position, float rotation, float lifetime)
{
    mLifeTime = lifetime;
    mIsDead = false;
    mState = ActorState::Active;
    mDrawComponent->SetVisible(true);
    mPosition = position;
    mRotation = rotation;
}

void Particle::OnUpdate(float deltaTime)
{
    mLifeTime -= deltaTime;
    if (mLifeTime < 0.0f) {
        mLifeTime = 0.0f;
        Kill();
        return;
    }
}

ParticleSystemComponent::ParticleSystemComponent(class Actor* owner, std::vector<Vector2> &vertices, int poolSize, int updateOrder,
    SystemType type, Vector3 color, bool filled)
    : Component(owner, updateOrder)
    ,mSystemType(type)
    ,mParticleColor(color)
    ,mParticleFilled(filled)
{
    for (int i = 0; i < poolSize; i++) {
        auto particle = new Particle(mOwner->GetGame(), vertices, type, color, filled);
        mParticles.emplace_back(particle);
    }
}

void ParticleSystemComponent::EmitParticle(float lifetime, float speed, const Vector2& offsetPosition)
{
    if (mSystemType == SystemType::Explosion) {
        for (auto particle : mParticles) {
            Vector2 newPosition = offsetPosition + mOwner->GetPosition();
            particle->Awake(newPosition, mOwner->GetRotation(), lifetime);

            Vector2 force;
            force.x = Random::GetFloatRange(-speed, speed);
            force.y = Random::GetFloatRange(-speed, speed);
            particle->GetComponent<RigidBodyComponent>()->ApplyForce(force);
        }
        return;
    } else if (mSystemType == SystemType::Fire) {
        for (auto particle : mParticles) {
            Vector2 newPosition = offsetPosition + mOwner->GetPosition();
            particle->Awake(newPosition, mOwner->GetRotation(), lifetime);

            float noise = Random::GetFloatRange(-1.0f, 1.0f);

            Vector2 force;
            force.x = -speed * Math::Cos(mOwner->GetRotation() + noise);
            force.y = -speed * Math::Sin(mOwner->GetRotation() + noise);
            particle->GetComponent<RigidBodyComponent>()->ApplyForce(force);
        }
        return;
    }

    for (auto particle : mParticles) {
        if (particle->IsDead()) {
            Vector2 newPosition = offsetPosition + mOwner->GetPosition();
            particle->Awake(newPosition, mOwner->GetRotation(), lifetime);

            // Para nave vermelha, velocidade maior (1.5x)
            float actualSpeed = mParticleFilled ? speed * 1.5f : speed;
            Vector2 shootForce;
            shootForce.x = actualSpeed * Math::Cos(mOwner->GetRotation());
            shootForce.y = actualSpeed * Math::Sin(mOwner->GetRotation());
            particle->GetComponent<RigidBodyComponent>()->ApplyForce(shootForce);
            return;
        }
    }
}