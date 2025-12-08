#pragma once
#include "Component.h"
#include "DrawComponent.h"
#include "../Math.h"
#include <vector>

class LaserBeamComponent;

class LaserDrawComponent : public DrawComponent
{
public:
    LaserDrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color, class LaserBeamComponent* laserComp = nullptr);
    void Draw(Renderer* renderer) override;
    
    void SetAlpha(float alpha) { mAlpha = alpha; }
    float GetAlpha() const { return mAlpha; }

private:
    void DrawLaserFlare(Renderer* renderer, const Vector2& position, Vector3 color, float alpha, float size);
    void DrawLaserImpact(Renderer* renderer, const Vector2& position, Vector3 color, float alpha);
    
    float mAlpha;
    class LaserBeamComponent* mLaserComponent;
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
    void Activate(const Vector2& startPos, float rotation, float screenWidth, float screenHeight, class Ship* ownerShip = nullptr);
    bool IntersectCircle(const Vector2& circleCenter, float radius) const;
    
    Vector2 GetStartPos() const { return mStartPos; }
    Vector2 GetEndPos() const { return mEndPos; }
    bool HitObject() const { return mHitObject; }

private:
    Vector3 mColor;
    float mLifetime;
    float mMaxLifetime;
    bool mIsActive;
    Vector2 mStartPos;
    Vector2 mEndPos;
    float mRotation;
    bool mHitObject; // Indica se o raio atingiu um objeto (não apenas a borda)
    
    class LaserDrawComponent* mDrawComponent;
    std::vector<Vector2> CreateLineVertices(float width);
    void CalculateEndPoint(float screenWidth, float screenHeight, class Ship* ownerShip = nullptr);
    
    // Ray casting: calcula a distância até a interseção com um círculo
    // Retorna -1 se não houver interseção, ou a distância se houver
    float RayCastToCircle(const Vector2& rayStart, const Vector2& rayDir, 
                          const Vector2& circleCenter, float radius) const;
};

