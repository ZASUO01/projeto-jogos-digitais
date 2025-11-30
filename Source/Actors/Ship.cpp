//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Ship.h"
#include "../Game.h"
#include "../Components/CircleColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponent.h"
#include "../Components/ParticleSystemComponent.h"

Ship::Ship(Game* game,
           const float height,
           const float forwardForce,
           const float rotationForce)
        : Actor(game)
        , mForwardSpeed(forwardForce)
        , mRotationForce(rotationForce)
        , mLaserCooldown(0.f)
        , mHeight(height)
        , mBurnCooldown(0.f)
        , mDrawComponent(nullptr)
        , mRigidBodyComponent(nullptr)
        , mCircleColliderComponent(nullptr)
        , mWeapon(nullptr)
        , mTurbine(nullptr)
{
    std::vector<Vector2> vertices = CreateShipVertices();
    mDrawComponent = new DrawComponent(this, vertices, 100, Vector3(0, 0, 1));
    mRigidBodyComponent = new RigidBodyComponent(this);
    mCircleColliderComponent = new CircleColliderComponent(this, mHeight / 3);

    std::vector<Vector2> particleVertices = CreateParticleVertices(1.0f);
    mWeapon = new ParticleSystemComponent(this, particleVertices, 20);

    std::vector<Vector2> turbineVertices = CreateParticleVertices(1.0f);
    mTurbine = new ParticleSystemComponent(this, turbineVertices, 20, 10, SystemType::Fire);
}

void Ship::OnProcessInput(const uint8_t* state)
{
    if (state[SDL_SCANCODE_W]) {
        /*
        Vector2 force;
        force.x = mForwardSpeed * Math::Cos(mRotation);
        force.y = mForwardSpeed * Math::Sin(mRotation);

        mRigidBodyComponent->ApplyForce(force);

        if (mBurnCooldown <= 0.f) {
            mTurbine->EmitParticle(0.1f, 25000);
            mBurnCooldown = 0.2f;
        }
        */
        mPosition.y -= 10.0f;
    }

    if (state[SDL_SCANCODE_A]) {
        //mRigidBodyComponent->SetAngularSpeed(-mRotationForce);
        mPosition.x -= 10.0f;
    }

    if (state[SDL_SCANCODE_D]) {
        //mRigidBodyComponent->SetAngularSpeed(mRotationForce);

        mPosition.x += 10.0f;
    }

    if (state[SDL_SCANCODE_S]) {
        mPosition.y += 10.0f;
    }

    /*
    else if (state[SDL_SCANCODE_SPACE]) {
        if (mLaserCooldown <= 0.f) {
            mWeapon->EmitParticle(1  , 16000);
            mLaserCooldown = 0.2f;
        }
    }

    else {
        mRigidBodyComponent->SetAngularSpeed(0);
    }
    */
}

void Ship::OnUpdate(float deltaTime)
{
    mLaserCooldown -= deltaTime;
    if (mLaserCooldown <= 0) {
        mLaserCooldown = 0.f;
    }

    mBurnCooldown -= deltaTime;
    if (mBurnCooldown <= 0) {
        mBurnCooldown = 0.f;
    }
}

std::vector<Vector2> Ship::CreateShipVertices() {
    std::vector<Vector2> vertices;

    float step = mHeight / 4;
    vertices.emplace_back(Vector2(2 * step, 0.0));
    vertices.emplace_back(Vector2(2 * -step, 2 * step));
    vertices.emplace_back(Vector2(-step, step));
    vertices.emplace_back(Vector2(-step, -step));
    vertices.emplace_back(Vector2(2* -step, 2 * -step));

    return vertices;
}

std::vector<Vector2> Ship::CreateParticleVertices(float size) {
    std::vector<Vector2> vertices;

    vertices.emplace_back(Vector2(-size, -size));
    vertices.emplace_back(Vector2(size, -size));
    vertices.emplace_back(Vector2(size, size));
    vertices.emplace_back(Vector2(-size, size));

    return vertices;
}
