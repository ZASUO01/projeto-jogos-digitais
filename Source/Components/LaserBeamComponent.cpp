#include "LaserBeamComponent.h"
#include "../Game.h"
#include "../Components/DrawComponent.h"
#include "../Components/CircleColliderComponent.h"
#include "../Renderer/Renderer.h"
#include "../Math.h"
#include "../Actors/Ship.h"

LaserDrawComponent::LaserDrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color, class LaserBeamComponent* laserComp)
    : DrawComponent(owner, vertices, drawOrder, color, true)
    , mAlpha(1.0f)
    , mLaserComponent(laserComp)
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
    
    Vector2 baseScale = mOwner->GetScale();
    float currentTime = SDL_GetTicks() / 1000.0f;
    
    // Efeito de pulso sutil para simular energia do laser
    float pulse = 0.95f + Math::Sin(currentTime * 20.0f) * 0.05f;
    
    // Camada externa - Halo brilhante muito largo (efeito de bloom)
    Vector2 outerGlowScale = baseScale;
    outerGlowScale.y *= 6.0f;
    Matrix4 outerGlowTransform = Matrix4::CreateScale(outerGlowScale.x, outerGlowScale.y, 1.0f);
    outerGlowTransform = outerGlowTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    outerGlowTransform = outerGlowTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    float outerGlowAlpha = mAlpha * 0.15f * pulse;
    renderer->DrawFilledWithAlpha(outerGlowTransform, vertexArray, mColor, outerGlowAlpha);
    
    // Camada de glow média - Brilho intermediário
    Vector2 glowScale = baseScale;
    glowScale.y *= 4.0f;
    Matrix4 glowTransform = Matrix4::CreateScale(glowScale.x, glowScale.y, 1.0f);
    glowTransform = glowTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    glowTransform = glowTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    float glowAlpha = mAlpha * 0.35f * pulse;
    renderer->DrawFilledWithAlpha(glowTransform, vertexArray, mColor, glowAlpha);
    
    // Camada média - Brilho mais intenso
    Vector2 midScale = baseScale;
    midScale.y *= 2.5f;
    Matrix4 midTransform = Matrix4::CreateScale(midScale.x, midScale.y, 1.0f);
    midTransform = midTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    midTransform = midTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    float midAlpha = mAlpha * 0.7f * pulse;
    renderer->DrawFilledWithAlpha(midTransform, vertexArray, mColor, midAlpha);
    
    // Core do laser - Linha central brilhante e intensa
    float coreAlpha = mAlpha * pulse;
    renderer->DrawFilledWithAlpha(mOwner->GetModelMatrix(), vertexArray, mColor, coreAlpha);
    
    // Adicionar linha branca brilhante no centro para efeito de "hot core"
    Vector3 whiteCore(1.0f, 1.0f, 1.0f);
    Vector2 coreScale = baseScale;
    coreScale.y *= 0.3f; // Muito fino no centro
    Matrix4 coreTransform = Matrix4::CreateScale(coreScale.x, coreScale.y, 1.0f);
    coreTransform = coreTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    coreTransform = coreTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    float whiteCoreAlpha = mAlpha * 0.8f * pulse;
    renderer->DrawFilledWithAlpha(coreTransform, vertexArray, whiteCore, whiteCoreAlpha);
    
    // Desenhar flare no início do raio (ponto de origem - efeito de "muzzle flash")
    Vector2 startPos = mLaserComponent ? mLaserComponent->GetStartPos() : mOwner->GetPosition();
    DrawLaserFlare(renderer, startPos, mColor, mAlpha * pulse, 12.0f);
    
    // Desenhar efeito de impacto no final do raio se atingiu um objeto
    if (mLaserComponent && mLaserComponent->HitObject()) {
        Vector2 endPos = mLaserComponent->GetEndPos();
        DrawLaserImpact(renderer, endPos, mColor, mAlpha * pulse);
    }
}

void LaserDrawComponent::DrawLaserFlare(Renderer* renderer, const Vector2& position, Vector3 color, float alpha, float size)
{
    // Criar vértices para um círculo (flare no início do raio)
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
    
    // Desenhar múltiplas camadas para efeito de brilho intenso
    // Camada externa muito grande e suave
    Matrix4 outerFlareTransform = Matrix4::CreateScale(2.5f, 2.5f, 1.0f) * 
                                   Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(outerFlareTransform, flareArray, color, alpha * 0.2f);
    
    // Camada média
    Matrix4 midFlareTransform = Matrix4::CreateScale(1.8f, 1.8f, 1.0f) * 
                                 Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(midFlareTransform, flareArray, color, alpha * 0.4f);
    
    // Camada interna brilhante
    Matrix4 innerFlareTransform = Matrix4::CreateScale(1.2f, 1.2f, 1.0f) * 
                                  Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(innerFlareTransform, flareArray, color, alpha * 0.7f);
    
    // Core branco brilhante
    Vector3 whiteCore(1.0f, 1.0f, 1.0f);
    Matrix4 coreFlareTransform = Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(coreFlareTransform, flareArray, whiteCore, alpha * 0.9f);
    
    delete flareArray;
}

void LaserDrawComponent::DrawLaserImpact(Renderer* renderer, const Vector2& position, Vector3 color, float alpha)
{
    // Criar vértices para um círculo no ponto de impacto
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
    
    // Criar VertexArray temporário para o impacto
    class VertexArray* impactArray = new VertexArray(floatVertices.data(), 
                                                      static_cast<unsigned int>(floatVertices.size()), 
                                                      indices.data(), 
                                                      static_cast<unsigned int>(indices.size()));
    
    // Desenhar múltiplas camadas para efeito de flash de impacto intenso
    // Camada externa muito grande (explosão de luz)
    Matrix4 outerImpactTransform = Matrix4::CreateScale(3.0f, 3.0f, 1.0f) * 
                                    Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(outerImpactTransform, impactArray, color, alpha * 0.25f);
    
    // Camada grande
    Matrix4 largeImpactTransform = Matrix4::CreateScale(2.2f, 2.2f, 1.0f) * 
                                   Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(largeImpactTransform, impactArray, color, alpha * 0.45f);
    
    // Camada média
    Matrix4 midImpactTransform = Matrix4::CreateScale(1.5f, 1.5f, 1.0f) * 
                                  Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(midImpactTransform, impactArray, color, alpha * 0.7f);
    
    // Camada interna brilhante
    Matrix4 innerImpactTransform = Matrix4::CreateScale(1.1f, 1.1f, 1.0f) * 
                                   Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(innerImpactTransform, impactArray, color, alpha * 0.9f);
    
    // Core branco super brilhante (ponto de impacto intenso)
    Vector3 whiteCore(1.0f, 1.0f, 1.0f);
    Matrix4 coreImpactTransform = Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));
    renderer->DrawFilledWithAlpha(coreImpactTransform, impactArray, whiteCore, alpha);
    
    delete impactArray;
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
    
    // Calcular pulso animado (mais rápido e sutil que o do chão)
    // Chão usa: sin(time * 2.0) com variação 0.7-1.0
    // Aqui usamos: sin(time * 3.5) com variação mais rápida e sutil
    float currentTime = SDL_GetTicks() / 1000.0f;
    float pulseSin = Math::Sin(currentTime * 3.5f); // Varia de -1 a 1
    float pulse = 0.8f + (pulseSin + 1.0f) * 0.15f; // Varia entre 0.8 e 0.95
    
    Vector2 baseScale = mOwner->GetScale();
    
    Vector2 glowScale = baseScale;
    glowScale.x *= 1.3f;
    glowScale.y *= 1.3f;
    
    Matrix4 glowTransform = Matrix4::CreateScale(glowScale.x, glowScale.y, 1.0f);
    glowTransform = glowTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    glowTransform = glowTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    
    // Aplicar pulso ao alpha do glow (varia de 0.28 a 0.38 - mais visível)
    float glowAlpha = 0.28f + (pulse - 0.8f) * 0.67f; // Mapeia 0.8-0.95 para 0.28-0.38
    renderer->DrawFilledWithAlpha(glowTransform, vertexArray, mColor, glowAlpha);
    
    Vector2 midScale = baseScale;
    midScale.x *= 1.15f;
    midScale.y *= 1.15f;
    Matrix4 midTransform = Matrix4::CreateScale(midScale.x, midScale.y, 1.0f);
    midTransform = midTransform * Matrix4::CreateRotationZ(mOwner->GetRotation());
    midTransform = midTransform * Matrix4::CreateTranslation(Vector3(mOwner->GetPosition().x, mOwner->GetPosition().y, 0.0f));
    
    // Aplicar pulso ao alpha do meio (varia de 0.56 a 0.665 - mais visível)
    float midAlpha = 0.56f + (pulse - 0.8f) * 0.7f; // Mapeia 0.8-0.95 para 0.56-0.665
    renderer->DrawFilledWithAlpha(midTransform, vertexArray, mColor, midAlpha);
    
    // Core mantém alpha fixo para contraste
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
    , mHitObject(false)
    , mDrawComponent(nullptr)
{
    std::vector<Vector2> vertices = CreateLineVertices(3.0f);
    mDrawComponent = new LaserDrawComponent(mOwner, vertices, 98, mColor, this);
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

float LaserBeamComponent::RayCastToCircle(const Vector2& rayStart, const Vector2& rayDir, 
                                          const Vector2& circleCenter, float radius) const
{
    // Vetor do início do raio até o centro do círculo
    Vector2 toCircle = circleCenter - rayStart;
    
    // Projeção do vetor toCircle na direção do raio
    float projection = toCircle.x * rayDir.x + toCircle.y * rayDir.y;
    
    // Se a projeção for negativa, o círculo está atrás do raio
    if (projection < 0.0f) {
        return -1.0f;
    }
    
    // Ponto mais próximo do círculo na linha do raio
    Vector2 closestPoint;
    closestPoint.x = rayStart.x + rayDir.x * projection;
    closestPoint.y = rayStart.y + rayDir.y * projection;
    
    // Distância do ponto mais próximo até o centro do círculo
    Vector2 toClosest = circleCenter - closestPoint;
    float distSq = toClosest.x * toClosest.x + toClosest.y * toClosest.y;
    float radiusSq = radius * radius;
    
    // Se a distância for maior que o raio, não há interseção
    if (distSq > radiusSq) {
        return -1.0f;
    }
    
    // Calcular a distância até o ponto de entrada do raio no círculo
    float halfChord = Math::Sqrt(radiusSq - distSq);
    float distToIntersection = projection - halfChord;
    
    // Retornar a distância (sempre positiva pois projection >= 0)
    return distToIntersection;
}

void LaserBeamComponent::CalculateEndPoint(float screenWidth, float screenHeight, class Ship* ownerShip)
{
    float cosR = Math::Cos(mRotation);
    float sinR = Math::Sin(mRotation);
    Vector2 rayDir(cosR, sinR);
    
    // Primeiro, calcular a distância até as bordas da tela
    float rightEdge = screenWidth;
    float leftEdge = 0.0f;
    float topEdge = 0.0f;
    float bottomEdge = screenHeight;
    
    float minDist = screenWidth + screenHeight;
    float t;
    
    // Verificar interseção com borda direita
    if (Math::Abs(cosR) > 0.0001f) {
        t = (rightEdge - mStartPos.x) / cosR;
        if (t > 0.0f) {
            float y = mStartPos.y + sinR * t;
            if (y >= topEdge && y <= bottomEdge) {
                minDist = Math::Min(minDist, t);
            }
        }
    }
    
    // Verificar interseção com borda esquerda
    if (Math::Abs(cosR) > 0.0001f) {
        t = (leftEdge - mStartPos.x) / cosR;
        if (t > 0.0f) {
            float y = mStartPos.y + sinR * t;
            if (y >= topEdge && y <= bottomEdge) {
                minDist = Math::Min(minDist, t);
            }
        }
    }
    
    // Verificar interseção com borda superior
    if (Math::Abs(sinR) > 0.0001f) {
        t = (topEdge - mStartPos.y) / sinR;
        if (t > 0.0f) {
            float x = mStartPos.x + cosR * t;
            if (x >= leftEdge && x <= rightEdge) {
                minDist = Math::Min(minDist, t);
            }
        }
    }
    
    // Verificar interseção com borda inferior
    if (Math::Abs(sinR) > 0.0001f) {
        t = (bottomEdge - mStartPos.y) / sinR;
        if (t > 0.0f) {
            float x = mStartPos.x + cosR * t;
            if (x >= leftEdge && x <= rightEdge) {
                minDist = Math::Min(minDist, t);
            }
        }
    }
    
    // Guardar distância até a borda para comparação
    float edgeDist = minDist;
    
    // Se não encontrou borda válida, usar distância máxima
    if (minDist >= screenWidth + screenHeight) {
        minDist = Math::Sqrt(screenWidth * screenWidth + screenHeight * screenHeight);
        edgeDist = minDist;
    }
    
    // Agora fazer ray casting para verificar colisões com objetos
    mHitObject = false;
    class Game* game = mOwner->GetGame();
    if (game) {
        // Verificar Ship1
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
        
        // Verificar Ship2
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
    
    // Calcular ponto final baseado na distância mínima encontrada
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

