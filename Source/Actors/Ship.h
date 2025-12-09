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
    ~Ship();

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    
    int GetLives() const { return mLives; }
    void SetLives(int lives) { 
        if (mLives != lives) {
            mLives = lives; 
            UpdateLivesDisplay();
        }
    }
    void TakeDamage();
    
    bool IsInvincible() const { return mInvincibilityTimer > 0.0f; }
    bool IsRedShip() const { return mIsRedShip; }

private:
    enum class Direction {
        Right,
        UpRight,
        Down,
        DownRight,
        DownLeft,
        Left,
        UpLeft,
        Up
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
    class DrawComponent* mColliderDrawComponent;
    class Actor* mLivesActors[4];
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
    class TrailComponent* mTrailComponent;
    class AudioPlayer* mHitSound;

    std::vector<Vector2> CreateShipVertices();
    std::vector<Vector2> CreateParticleVertices(float size);
    std::vector<Vector2> CreateLifeSquareVertices();
    std::vector<Vector2> CreateColliderCircleVertices(float radius);
    void UpdateLivesDisplay();
    
    Direction GetClosestDirection(float rotation) const;
    float DirectionToRadians(Direction dir) const;
    Direction GetNextDirection(Direction current, bool clockwise) const;
};