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
                  float rotationForce = 5.0,
                  Vector3 color = Vector3(0.0f, 1.0f, 1.0f),
                  bool isRedShip = false);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    
    int GetLives() const { return mLives; }
    void SetLives(int lives) { 
        if (mLives != lives) {
            mLives = lives; 
            UpdateLivesDisplay();
        }
    }
    void TakeDamage() { 
        if (mLives > 0 && mInvincibilityTimer <= 0.0f) {
            mLives--; 
            mInvincibilityTimer = 2.0f; // 2 segundos de invencibilidade
            UpdateLivesDisplay();
        }
    }
    
    bool IsInvincible() const { return mInvincibilityTimer > 0.0f; }

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
    float mRotationCooldown;
    float mInvincibilityTimer;
    int mLives;
    Vector3 mShipColor;
    bool mIsRedShip;

    Vector2 mTarget;

    class DrawComponent* mDrawComponent;
    class DrawComponent* mColliderDrawComponent; // Círculo de colisão visível com glow
    class Actor* mLivesActors[3]; // Actors filhos para os quadrados de vida
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;

    std::vector<Vector2> CreateShipVertices();
    std::vector<Vector2> CreateParticleVertices(float size);
    std::vector<Vector2> CreateLifeSquareVertices();
    std::vector<Vector2> CreateColliderCircleVertices(float radius);
    void UpdateLivesDisplay();
    
    // Converte ângulo para a direção mais próxima
    Direction GetClosestDirection(float rotation) const;
    
    // Converte direção para ângulo em radianos
    float DirectionToRadians(Direction dir) const;
    
    // Obtém a próxima direção na sequência (horário ou anti-horário)
    Direction GetNextDirection(Direction current, bool clockwise) const;
};