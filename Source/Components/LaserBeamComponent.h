#pragma once
#include "Component.h"
#include "DrawComponent.h"
#include "../Math.h"
#include <vector>

class LaserDrawComponent : public DrawComponent
{
public:
    LaserDrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color);
    void Draw(Renderer* renderer) override;
    
    void SetAlpha(float alpha) { mAlpha = alpha; }
    float GetAlpha() const { return mAlpha; }

private:
    float mAlpha;
};

class ColliderDrawComponent : public DrawComponent
{
public:
    ColliderDrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color);
    void Draw(Renderer* renderer) override;
};

class LaserBeamComponent : public Component
{
public:
    LaserBeamComponent(class Actor* owner, Vector3 color, float lifetime = 0.5f);
    ~LaserBeamComponent();

    void Update(float deltaTime) override;
    
    bool IsActive() const { return mIsActive && mLifetime > 0.0f; }
    float GetAlpha() const { return mLifetime / mMaxLifetime; }
    void Activate(const Vector2& startPos, float rotation, float screenWidth, float screenHeight);
    bool IntersectCircle(const Vector2& circleCenter, float radius) const;

private:
    Vector3 mColor;
    float mLifetime;
    float mMaxLifetime;
    bool mIsActive;
    Vector2 mStartPos;
    Vector2 mEndPos;
    float mRotation;
    
    class LaserDrawComponent* mDrawComponent;
    std::vector<Vector2> CreateLineVertices(float width);
    void CalculateEndPoint(float screenWidth, float screenHeight);
};

