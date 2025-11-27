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
    enum class Direction {
        Right,          // 0°
        UpRight,        // 30°
        Down,           // 90°
        DownRight,      // -30° ou 330°
        DownLeft,       // 150°
        Left,           // 180°
        UpLeft,         // 210°
        Up              // 270° ou -90°
    };

    float mForwardSpeed;
    float mRotationForce;
    float mLaserCooldown;
    float mHeight;
    float mSpawnPointTimer;
    float mBurnCooldown;
    float mRotationCooldown;

    Vector2 mTarget;

    class DrawComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
    class ParticleSystemComponent* mWeapon;
    class ParticleSystemComponent* mTurbine;

    std::vector<Vector2> CreateShipVertices();
    std::vector<Vector2> CreateParticleVertices(float size);
    
    // Converte ângulo para a direção mais próxima
    Direction GetClosestDirection(float rotation) const;
    
    // Converte direção para ângulo em radianos
    float DirectionToRadians(Direction dir) const;
    
    // Obtém a próxima direção na sequência (horário ou anti-horário)
    Direction GetNextDirection(Direction current, bool clockwise) const;
};