//
// Created by Lucas N. Ferreira on 08/09/23.
//

#pragma once
#include "Actor.h"

class Ship : public Actor
{
public:
    explicit Ship(Game* game, float height,
                  float forwardForce = 500.0f,
                  float rotationForce = 5.0);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:
    float mForwardSpeed;
    float mRotationForce;
    float mLaserCooldown;
    float mHeight;
    float mSpawnPointTimer;
    float mBurnCooldown;

    Vector2 mTarget;

    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
    class ParticleSystemComponent* mWeapon;
    class ParticleSystemComponent* mTurbine;

    std::vector<Vector2> CreateShipVertices();
    std::vector<Vector2> CreateParticleVertices(float size);
};