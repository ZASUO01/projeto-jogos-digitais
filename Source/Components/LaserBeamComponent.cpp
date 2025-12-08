#include "LaserBeamComponent.h"
#include "../Game.h"
#include "../Components/DrawComponent.h"
#include "../Renderer/Renderer.h"
#include "../Math.h"

LaserDrawComponent::LaserDrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color)
    : DrawComponent(owner, vertices, drawOrder, color, true)
    , mAlpha(1.0f)
{
}

void LaserDrawComponent::Draw(Renderer* renderer)
{
    if (mOwner->GetState() != ActorState::Active || !mIsVisible) {
        return;
    }
    
    class VertexArray* vertexArray = GetVertexArray();
    if (!vertexArray) {
        return;
    }

    const Vector2 baseScale = mOwner->GetScale();
    
    Vector2 glowScale = baseScale;
    glowScale.y *= 3.0f;
    
    Matrix4 glowTransform = Matrix4::CreateScale(glowScale.x, glowScale.y, 1.0f);
    glowTransform = glowTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    glowTransform = glowTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    
    float glowAlpha = mAlpha * 0.3f;
    renderer->DrawFilledWithAlpha(glowTransform, vertexArray, mColor, glowAlpha);
    
    Vector2 midScale = baseScale;
    midScale.y *= 2.0f;
    Matrix4 midTransform = Matrix4::CreateScale(midScale.x, midScale.y, 1.0f);
    midTransform = midTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    midTransform = midTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    
    float midAlpha = mAlpha * 0.6f;
    renderer->DrawFilledWithAlpha(midTransform, vertexArray, mColor, midAlpha);
    
    renderer->DrawFilledWithAlpha(mOwner->GetModelMatrix(), vertexArray, mColor, mAlpha);
}

ColliderDrawComponent::ColliderDrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color)
    : DrawComponent(owner, vertices, drawOrder, color, false)
{
}

void ColliderDrawComponent::Draw(Renderer* renderer)
{
    if (mOwner->GetState() != ActorState::Active || !mIsVisible) {
        return;
    }
    
    class VertexArray* vertexArray = GetVertexArray();
    if (!vertexArray) {
        return;
    }
    
    Vector2 baseScale = mOwner->GetScale();
    
    Vector2 glowScale = baseScale;
    glowScale.x *= 1.3f;
    glowScale.y *= 1.3f;
    
    Matrix4 glowTransform = Matrix4::CreateScale(glowScale.x, glowScale.y, 1.0f);
    glowTransform = glowTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    glowTransform = glowTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    
    float glowAlpha = 0.4f;
    renderer->DrawFilledWithAlpha(glowTransform, vertexArray, mColor, glowAlpha);
    
    Vector2 midScale = baseScale;
    midScale.x *= 1.15f;
    midScale.y *= 1.15f;
    Matrix4 midTransform = Matrix4::CreateScale(midScale.x, midScale.y, 1.0f);
    midTransform = midTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    midTransform = midTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    
    float midAlpha = 0.7f;
    renderer->DrawFilledWithAlpha(midTransform, vertexArray, mColor, midAlpha);
    
    float coreAlpha = 1.0f;
    renderer->DrawFilledWithAlpha(mOwner->GetModelMatrix(), vertexArray, mColor, coreAlpha);
    renderer->Draw(mOwner->GetModelMatrix(), vertexArray, mColor);
}

LaserBeamComponent::LaserBeamComponent(class Actor* owner, Vector3 color, float lifetime)
    : Component(owner, 10)
    , mColor(color)
    , mLifetime(0.0f)
    , mMaxLifetime(lifetime)
    , mIsActive(false)
    , mStartPos(Vector2::Zero)
    , mEndPos(Vector2::Zero)
    , mRotation(0.0f)
    , mDrawComponent(nullptr)
{
    std::vector<Vector2> vertices = CreateLineVertices(3.0f);
    mDrawComponent = new LaserDrawComponent(mOwner, vertices, 98, mColor);
    mDrawComponent->SetVisible(false);
}

LaserBeamComponent::~LaserBeamComponent()
{
}

void LaserBeamComponent::Update(float deltaTime)
{
    if (mIsActive && mLifetime > 0.0f) {
        mLifetime -= deltaTime;
        
        if (mLifetime <= 0.0f) {
            mLifetime = 0.0f;
            mIsActive = false;
            mDrawComponent->SetVisible(false);
        } else {
            float alpha = GetAlpha();
            mDrawComponent->SetAlpha(alpha);
            mDrawComponent->SetVisible(true);
        }
    }
}

void LaserBeamComponent::Activate(const Vector2& startPos, float rotation, float screenWidth, float screenHeight)
{
    mStartPos = startPos;
    mRotation = rotation;
    mLifetime = mMaxLifetime;
    mIsActive = true;
    
    CalculateEndPoint(screenWidth, screenHeight);
    
    mOwner->SetPosition(mStartPos);
    mOwner->SetRotation(mRotation);
    
    Vector2 direction = mEndPos - mStartPos;
    float length = direction.Length();
    
    mOwner->SetScale(Vector2(length, 1.0f));
    
    mDrawComponent->SetVisible(true);
}

void LaserBeamComponent::CalculateEndPoint(float screenWidth, float screenHeight)
{
    float cosR = Math::Cos(mRotation);
    float sinR = Math::Sin(mRotation);
    
    float rightEdge = screenWidth;
    float leftEdge = 0.0f;
    float topEdge = 0.0f;
    float bottomEdge = screenHeight;
    
    float minDist = screenWidth + screenHeight;
    float t;
    
    if (Math::Abs(cosR) > 0.0001f) {
        t = (rightEdge - mStartPos.x) / cosR;
        if (t > 0.0f) {
            float y = mStartPos.y + sinR * t;
            if (y >= topEdge && y <= bottomEdge) {
                minDist = Math::Min(minDist, t);
            }
        }
    }
    
    if (Math::Abs(cosR) > 0.0001f) {
        t = (leftEdge - mStartPos.x) / cosR;
        if (t > 0.0f) {
            float y = mStartPos.y + sinR * t;
            if (y >= topEdge && y <= bottomEdge) {
                minDist = Math::Min(minDist, t);
            }
        }
    }
    
    if (Math::Abs(sinR) > 0.0001f) {
        t = (topEdge - mStartPos.y) / sinR;
        if (t > 0.0f) {
            float x = mStartPos.x + cosR * t;
            if (x >= leftEdge && x <= rightEdge) {
                minDist = Math::Min(minDist, t);
            }
        }
    }
    
    if (Math::Abs(sinR) > 0.0001f) {
        t = (bottomEdge - mStartPos.y) / sinR;
        if (t > 0.0f) {
            float x = mStartPos.x + cosR * t;
            if (x >= leftEdge && x <= rightEdge) {
                minDist = Math::Min(minDist, t);
            }
        }
    }
    
    if (minDist >= screenWidth + screenHeight) {
        minDist = Math::Sqrt(screenWidth * screenWidth + screenHeight * screenHeight);
    }
    
    mEndPos.x = mStartPos.x + cosR * minDist;
    mEndPos.y = mStartPos.y + sinR * minDist;
}

bool LaserBeamComponent::IntersectCircle(const Vector2& circleCenter, float radius) const
{
    if (!mIsActive || mLifetime <= 0.0f) {
        return false;
    }
    
    Vector2 lineDir = mEndPos - mStartPos;
    float lineLength = lineDir.Length();
    
    if (lineLength < 0.0001f) {
        return false;
    }
    
    lineDir.x /= lineLength;
    lineDir.y /= lineLength;
    
    Vector2 toCircle = circleCenter - mStartPos;
    float projection = toCircle.x * lineDir.x + toCircle.y * lineDir.y;
    
    projection = Math::Max(0.0f, Math::Min(lineLength, projection));
    
    Vector2 closestPoint;
    closestPoint.x = mStartPos.x + lineDir.x * projection;
    closestPoint.y = mStartPos.y + lineDir.y * projection;
    
    Vector2 toClosest = circleCenter - closestPoint;
    float distSq = toClosest.x * toClosest.x + toClosest.y * toClosest.y;
    
    return distSq <= (radius * radius);
}

std::vector<Vector2> LaserBeamComponent::CreateLineVertices( float width)
{
    std::vector<Vector2> vertices;
    const float halfWidth = width / 2.0f;
    vertices.emplace_back(0.0f, -halfWidth);
    vertices.emplace_back(1.0f, -halfWidth);
    vertices.emplace_back(1.0f, halfWidth);
    vertices.emplace_back(0.0f, halfWidth);
    return vertices;
}

