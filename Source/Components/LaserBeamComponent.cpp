#include "LaserBeamComponent.h"
#include "../Game.h"
#include "../Components/DrawComponent.h"
#include "../Components/CircleColliderComponent.h"
#include "../Renderer/Renderer.h"
#include "../Math.h"
#include "../Actors/Ship.h"

// Constrói o componente de desenho do laser
LaserDrawComponent::LaserDrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color, class LaserBeamComponent* laserComp)
    : DrawComponent(owner, vertices, drawOrder, color, true)
    , mAlpha(1.0f)
    ,     mLaserComponent(laserComp)
{
}

// Desenha o laser com múltiplas camadas de brilho e efeitos visuais
void LaserDrawComponent::Draw(Renderer* renderer)
{
    if (mOwner->GetState() != ActorState::Active || !mIsVisible) {
        return;
    }
    
    class VertexArray* vertexArray = GetVertexArray();
    if (!vertexArray) {
        return;
    }
    
    Vector2 baseScale = mOwner->GetScale();
    float currentTime = SDL_GetTicks() / 1000.0f;
    
    float pulse = 0.95f + Math::Sin(currentTime * 20.0f) * 0.05f;
    Vector2 outerGlowScale = baseScale;
    outerGlowScale.y *= 6.0f;
    Matrix4 outerGlowTransform = Matrix4::CreateScale(outerGlowScale.x, outerGlowScale.y, 1.0f);
    outerGlowTransform = outerGlowTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    outerGlowTransform = outerGlowTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    float outerGlowAlpha = mAlpha * 0.15f * pulse;
    renderer->DrawFilledWithAlpha(outerGlowTransform, vertexArray, mColor, outerGlowAlpha);
    Vector2 glowScale = baseScale;
    glowScale.y *= 4.0f;
    Matrix4 glowTransform = Matrix4::CreateScale(glowScale.x, glowScale.y, 1.0f);
    glowTransform = glowTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    glowTransform = glowTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    float glowAlpha = mAlpha * 0.35f * pulse;
    renderer->DrawFilledWithAlpha(glowTransform, vertexArray, mColor, glowAlpha);
    Vector2 midScale = baseScale;
    midScale.y *= 2.5f;
    Matrix4 midTransform = Matrix4::CreateScale(midScale.x, midScale.y, 1.0f);
    midTransform = midTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    midTransform = midTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    float midAlpha = mAlpha * 0.7f * pulse;
    renderer->DrawFilledWithAlpha(midTransform, vertexArray, mColor, midAlpha);
    float coreAlpha = mAlpha * pulse;
    renderer->DrawFilledWithAlpha(mOwner->GetModelMatrix(), vertexArray, mColor, coreAlpha);
    Vector3 whiteCore(1.0f, 1.0f, 1.0f);
    Vector2 coreScale = baseScale;
    coreScale.y *= 0.3f; // Muito fino no centro
    Matrix4 coreTransform = Matrix4::CreateScale(coreScale.x, coreScale.y, 1.0f);
    coreTransform = coreTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    coreTransform = coreTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    float whiteCoreAlpha = mAlpha * 0.8f * pulse;
    renderer->DrawFilledWithAlpha(coreTransform, vertexArray, whiteCore, whiteCoreAlpha);
    Vector2 startPos = mLaserComponent ? mLaserComponent->GetStartPos() : mOwner->GetPosition();
    DrawLaserFlare(renderer, startPos, mColor, mAlpha * pulse, 12.0f);
    if (mLaserComponent && mLaserComponent->HitObject()) {
        Vector2 endPos = mLaserComponent->GetEndPos();
        DrawLaserImpact(renderer, endPos, mColor, mAlpha * pulse);
    }
}

// Desenha um efeito de flare no início do laser
void LaserDrawComponent::DrawLaserFlare(Renderer* renderer, const Vector2& position, Vector3 color, float alpha, float size)
{
    const int numSegments = 16;
    std::vector<Vector2> flareVertices;
    
    for (int i = 0; i < numSegments; i++) {
        float angle = (Math::TwoPi * i) / numSegments;
        flareVertices.emplace_back(Vector2(
            position.x + Math::Cos(angle) * size,
            position.y + Math::Sin(angle) * size
        ));
    }
    
    // Converter para formato de float array
    std::vector<float> floatVertices;
    std::vector<unsigned int> indices;
    for (size_t i = 0; i < flareVertices.size(); ++i) {
        floatVertices.push_back(flareVertices[i].x);
        floatVertices.push_back(flareVertices[i].y);
        indices.push_back(static_cast<unsigned int>(i));
    }
    
    // Criar VertexArray temporário
    class VertexArray* flareArray = new VertexArray(floatVertices.data(), 
                                                    static_cast<unsigned int>(floatVertices.size()), 
                                                    indices.data(), 
                                                    static_cast<unsigned int>(indices.size()));
    
    Matrix4 outerFlareTransform = Matrix4::CreateScale(2.5f, 2.5f, 1.0f) * 
                                   Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(outerFlareTransform, flareArray, color, alpha * 0.2f);
    
    Matrix4 midFlareTransform = Matrix4::CreateScale(1.8f, 1.8f, 1.0f) * 
                                 Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(midFlareTransform, flareArray, color, alpha * 0.4f);
    
    Matrix4 innerFlareTransform = Matrix4::CreateScale(1.2f, 1.2f, 1.0f) * 
                                  Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(innerFlareTransform, flareArray, color, alpha * 0.7f);
    
    Vector3 whiteCore(1.0f, 1.0f, 1.0f);
    Matrix4 coreFlareTransform = Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(coreFlareTransform, flareArray, whiteCore, alpha * 0.9f);
    
    delete flareArray;
}

// Desenha um efeito de impacto quando o laser atinge um objeto
void LaserDrawComponent::DrawLaserImpact(Renderer* renderer, const Vector2& position, Vector3 color, float alpha)
{
    const float impactSize = 10.0f;
    const int numSegments = 16;
    std::vector<Vector2> impactVertices;
    
    for (int i = 0; i < numSegments; i++) {
        float angle = (Math::TwoPi * i) / numSegments;
        impactVertices.emplace_back(Vector2(
            position.x + Math::Cos(angle) * impactSize,
            position.y + Math::Sin(angle) * impactSize
        ));
    }
    
    // Converter para formato de float array
    std::vector<float> floatVertices;
    std::vector<unsigned int> indices;
    for (size_t i = 0; i < impactVertices.size(); ++i) {
        floatVertices.push_back(impactVertices[i].x);
        floatVertices.push_back(impactVertices[i].y);
        indices.push_back(static_cast<unsigned int>(i));
    }
    
    class VertexArray* impactArray = new VertexArray(floatVertices.data(), 
                                                      static_cast<unsigned int>(floatVertices.size()), 
                                                      indices.data(), 
                                                      static_cast<unsigned int>(indices.size()));
    
    Matrix4 outerImpactTransform = Matrix4::CreateScale(3.0f, 3.0f, 1.0f) * 
                                    Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(outerImpactTransform, impactArray, color, alpha * 0.25f);
    
    Matrix4 largeImpactTransform = Matrix4::CreateScale(2.2f, 2.2f, 1.0f) * 
                                   Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(largeImpactTransform, impactArray, color, alpha * 0.45f);
    
    Matrix4 midImpactTransform = Matrix4::CreateScale(1.5f, 1.5f, 1.0f) * 
                                  Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(midImpactTransform, impactArray, color, alpha * 0.7f);
    
    Matrix4 innerImpactTransform = Matrix4::CreateScale(1.1f, 1.1f, 1.0f) * 
                                   Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(innerImpactTransform, impactArray, color, alpha * 0.9f);
    
    Vector3 whiteCore(1.0f, 1.0f, 1.0f);
    Matrix4 coreImpactTransform = Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(coreImpactTransform, impactArray, whiteCore, alpha);
    
    delete impactArray;
}

// Constrói o componente de desenho do colisor
ColliderDrawComponent::ColliderDrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color)
    : DrawComponent(owner, vertices, drawOrder, color, false)
{
}

// Desenha o colisor com efeito de pulso animado
void ColliderDrawComponent::Draw(Renderer* renderer)
{
    if (mOwner->GetState() != ActorState::Active || !mIsVisible) {
        return;
    }
    
    class VertexArray* vertexArray = GetVertexArray();
    if (!vertexArray) {
        return;
    }
    
    float currentTime = SDL_GetTicks() / 1000.0f;
    float pulseSin = Math::Sin(currentTime * 3.5f);
    float pulse = 0.8f + (pulseSin + 1.0f) * 0.15f;
    
    Vector2 baseScale = mOwner->GetScale();
    
    Vector2 glowScale = baseScale;
    glowScale.x *= 1.3f;
    glowScale.y *= 1.3f;
    
    Matrix4 glowTransform = Matrix4::CreateScale(glowScale.x, glowScale.y, 1.0f);
    glowTransform = glowTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    glowTransform = glowTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    
    float glowAlpha = 0.28f + (pulse - 0.8f) * 0.67f;
    renderer->DrawFilledWithAlpha(glowTransform, vertexArray, mColor, glowAlpha);
    
    Vector2 midScale = baseScale;
    midScale.x *= 1.15f;
    midScale.y *= 1.15f;
    Matrix4 midTransform = Matrix4::CreateScale(midScale.x, midScale.y, 1.0f);
    midTransform = midTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    midTransform = midTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    
    float midAlpha = 0.56f + (pulse - 0.8f) * 0.7f;
    renderer->DrawFilledWithAlpha(midTransform, vertexArray, mColor, midAlpha);
    
    float coreAlpha = 1.0f;
    renderer->DrawFilledWithAlpha(mOwner->GetModelMatrix(), vertexArray, mColor, coreAlpha);
    renderer->Draw(mOwner->GetModelMatrix(), vertexArray, mColor);
}

// Constrói o componente de raio laser
LaserBeamComponent::LaserBeamComponent(class Actor* owner, Vector3 color, float lifetime)
    : Component(owner, 10)
    , mColor(color)
    , mLifetime(0.0f)
    , mMaxLifetime(lifetime)
    , mIsActive(false)
    , mStartPos(Vector2::Zero)
    , mEndPos(Vector2::Zero)
    , mRotation(0.0f)
    , mHitObject(false)
    , mDrawComponent(nullptr)
    , mHitShips()
{
    std::vector<Vector2> vertices = CreateLineVertices(3.0f);
    mDrawComponent = new LaserDrawComponent(mOwner, vertices, 98, mColor, this);
    mDrawComponent->SetVisible(false);
}

LaserBeamComponent::~LaserBeamComponent()
{
}

// Atualiza o tempo de vida do laser e sua visibilidade
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

// Ativa o laser na posição inicial com rotação especificada
void LaserBeamComponent::Activate(const Vector2& startPos, float rotation, float screenWidth, float screenHeight, class Ship* ownerShip)
{
    mStartPos = startPos;
    mRotation = rotation;
    mLifetime = mMaxLifetime;
    mIsActive = true;
    
    CalculateEndPoint(screenWidth, screenHeight, ownerShip);
    
    mOwner->SetPosition(mStartPos);
    mOwner->SetRotation(mRotation);
    
    Vector2 direction = mEndPos - mStartPos;
    float length = direction.Length();
    
    mOwner->SetScale(Vector2(length, 1.0f));
    
    mDrawComponent->SetVisible(true);
}

// Calcula a distância até a interseção de um raio com um círculo, retorna -1 se não houver interseção
float LaserBeamComponent::RayCastToCircle(const Vector2& rayStart, const Vector2& rayDir, 
                                          const Vector2& circleCenter, float radius) const
{
    Vector2 toCircle = circleCenter - rayStart;
    
    float projection = toCircle.x * rayDir.x + toCircle.y * rayDir.y;
    
    if (projection < 0.0f) {
        return -1.0f;
    }
    
    Vector2 closestPoint;
    closestPoint.x = rayStart.x + rayDir.x * projection;
    closestPoint.y = rayStart.y + rayDir.y * projection;
    
    Vector2 toClosest = circleCenter - closestPoint;
    float distSq = toClosest.x * toClosest.x + toClosest.y * toClosest.y;
    float radiusSq = radius * radius;
    
    if (distSq > radiusSq) {
        return -1.0f;
    }
    
    float halfChord = Math::Sqrt(radiusSq - distSq);
    float distToIntersection = projection - halfChord;
    
    return distToIntersection;
}

// Calcula o ponto final do laser considerando bordas da tela e colisões com naves
void LaserBeamComponent::CalculateEndPoint(float screenWidth, float screenHeight, class Ship* ownerShip)
{
    float cosR = Math::Cos(mRotation);
    float sinR = Math::Sin(mRotation);
    Vector2 rayDir(cosR, sinR);
    
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
    
    float edgeDist = minDist;
    
    if (minDist >= screenWidth + screenHeight) {
        minDist = Math::Sqrt(screenWidth * screenWidth + screenHeight * screenHeight);
        edgeDist = minDist;
    }
    
    mHitObject = false;
    class Game* game = mOwner->GetGame();
    if (game) {
        class Ship* ship1 = game->GetShip1();
        if (ship1 && ship1 != ownerShip && ship1->GetState() == ActorState::Active) {
            class CircleColliderComponent* collider = ship1->GetComponent<CircleColliderComponent>();
            if (collider) {
                float hitDist = RayCastToCircle(mStartPos, rayDir, ship1->GetPosition(), collider->GetRadius());
                if (hitDist > 0.0f && hitDist < minDist) {
                    minDist = hitDist;
                    mHitObject = true;
                }
            }
        }
        
        class Ship* ship2 = game->GetShip2();
        if (ship2 && ship2 != ownerShip && ship2->GetState() == ActorState::Active) {
            class CircleColliderComponent* collider = ship2->GetComponent<CircleColliderComponent>();
            if (collider) {
                float hitDist = RayCastToCircle(mStartPos, rayDir, ship2->GetPosition(), collider->GetRadius());
                if (hitDist > 0.0f && hitDist < minDist) {
                    minDist = hitDist;
                    mHitObject = true;
                }
            }
        }
    }
    
    mEndPos.x = mStartPos.x + cosR * minDist;
    mEndPos.y = mStartPos.y + sinR * minDist;
}

// Verifica se o laser intersecta com um círculo
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

// Cria vértices para uma linha retangular (usado para o laser)
std::vector<Vector2> LaserBeamComponent::CreateLineVertices(float width)
{
    std::vector<Vector2> vertices;
    float halfWidth = width / 2.0f;
    vertices.emplace_back(Vector2(0.0f, -halfWidth));
    vertices.emplace_back(Vector2(1.0f, -halfWidth));
    vertices.emplace_back(Vector2(1.0f, halfWidth));
    vertices.emplace_back(Vector2(0.0f, halfWidth));
    return vertices;
}

// Verifica se o laser já atingiu uma nave específica
bool LaserBeamComponent::HasHitShip(class Ship* ship) const
{
    if (!ship) {
        return false;
    }
    
    for (auto hitShip : mHitShips) {
        if (hitShip == ship) {
            return true;
        }
    }
    
    return false;
}

// Marca que uma nave foi atingida por este laser
void LaserBeamComponent::MarkShipHit(class Ship* ship)
{
    if (!ship) {
        return;
    }
    
    // Verificar se já foi marcada para evitar duplicatas
    if (!HasHitShip(ship)) {
        mHitShips.push_back(ship);
    }
}

