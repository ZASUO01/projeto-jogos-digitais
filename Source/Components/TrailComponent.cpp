#include "TrailComponent.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"
#include "../Math.h"
#include <algorithm>

// Vértices vazios para o construtor base (não usamos os vértices do DrawComponent)
static std::vector<Vector2> emptyTrailVertices;

TrailComponent::TrailComponent(class Actor* owner, Vector3 color, float trailLength, float updateInterval)
    : DrawComponent(owner, emptyTrailVertices, 96, color, false) // Draw order 96 (atrás da nave mas na frente do grid)
    , mTrailLength(trailLength)
    , mUpdateInterval(updateInterval)
    , mTimeSinceLastUpdate(0.0f)
{
    mTrailPoints.reserve(MAX_TRAIL_POINTS);
}

TrailComponent::~TrailComponent()
{
}

void TrailComponent::Update(float deltaTime)
{
    if (mOwner->GetState() != ActorState::Active) {
        return;
    }
    
    mTimeSinceLastUpdate += deltaTime;
    
    // Adicionar novo ponto periodicamente
    if (mTimeSinceLastUpdate >= mUpdateInterval) {
        AddTrailPoint(mOwner->GetPosition());
        mTimeSinceLastUpdate = 0.0f;
    }
    
    // Atualizar idade dos pontos e remover antigos
    UpdateTrailPoints(deltaTime);
    RemoveOldPoints();
}

void TrailComponent::AddTrailPoint(const Vector2& position)
{
    TrailPoint newPoint;
    newPoint.position = position;
    newPoint.age = 0.0f;
    newPoint.maxAge = mTrailLength;
    
    mTrailPoints.push_back(newPoint);
    
    // Limitar número de pontos
    if (mTrailPoints.size() > MAX_TRAIL_POINTS) {
        mTrailPoints.erase(mTrailPoints.begin());
    }
}

void TrailComponent::UpdateTrailPoints(float deltaTime)
{
    for (auto& point : mTrailPoints) {
        point.age += deltaTime;
    }
}

void TrailComponent::RemoveOldPoints()
{
    mTrailPoints.erase(
        std::remove_if(mTrailPoints.begin(), mTrailPoints.end(),
            [](const TrailPoint& point) {
                return point.age >= point.maxAge;
            }),
        mTrailPoints.end()
    );
}

void TrailComponent::Draw(Renderer* renderer)
{
    if (mOwner->GetState() != ActorState::Active || !mIsVisible) {
        return;
    }
    
    if (mTrailPoints.size() < 2) {
        return;
    }
    
    // Desenhar linhas conectando os pontos do rastro como quads (retângulos) para maior espessura
    // O alpha diminui conforme a idade do ponto aumenta
    const float trailWidth = 4.0f; // Largura do rastro
    
    for (size_t i = 0; i < mTrailPoints.size() - 1; ++i) {
        const TrailPoint& point1 = mTrailPoints[i];
        const TrailPoint& point2 = mTrailPoints[i + 1];
        
        // Calcular alpha baseado na idade (mais antigo = mais transparente)
        float ageRatio1 = point1.age / point1.maxAge;
        float ageRatio2 = point2.age / point2.maxAge;
        
        // Alpha vai de 1.0 (novo) a 0.0 (antigo)
        float alpha1 = 1.0f - ageRatio1;
        float alpha2 = 1.0f - ageRatio2;
        
        // Usar o alpha médio para o segmento
        float segmentAlpha = (alpha1 + alpha2) * 0.5f;
        
        // Calcular direção e perpendicular para criar um quad
        Vector2 direction = point2.position - point1.position;
        float length = direction.Length();
        if (length < 0.001f) {
            continue; // Pular pontos muito próximos
        }
        
        // Normalizar direção
        direction.x /= length;
        direction.y /= length;
        
        // Calcular perpendicular (rotacionar 90 graus)
        Vector2 perpendicular(-direction.y, direction.x);
        float halfWidth = trailWidth * 0.5f;
        
        // Criar vértices do quad (retângulo)
        Vector2 p1 = point1.position + perpendicular * halfWidth;
        Vector2 p2 = point1.position - perpendicular * halfWidth;
        Vector2 p3 = point2.position - perpendicular * halfWidth;
        Vector2 p4 = point2.position + perpendicular * halfWidth;
        
        std::vector<Vector2> quadVertices;
        quadVertices.push_back(p1);
        quadVertices.push_back(p2);
        quadVertices.push_back(p3);
        quadVertices.push_back(p4);
        
        // Converter para formato de float array
        std::vector<float> floatVertices;
        std::vector<unsigned int> indices;
        for (size_t j = 0; j < quadVertices.size(); ++j) {
            floatVertices.push_back(quadVertices[j].x);
            floatVertices.push_back(quadVertices[j].y);
        }
        
        // Índices para TRIANGLE_FAN (quad como fan de triângulos)
        // TRIANGLE_FAN desenha triângulos conectados a partir do primeiro vértice
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(3);
        
        // Criar VertexArray temporário
        class VertexArray* quadArray = new VertexArray(floatVertices.data(), 
                                                       static_cast<unsigned int>(floatVertices.size()), 
                                                       indices.data(), 
                                                       static_cast<unsigned int>(indices.size()));
        
        // Criar transformação de identidade (as posições já estão em coordenadas do mundo)
        Matrix4 identity = Matrix4::Identity;
        
        // Desenhar o quad preenchido com alpha
        renderer->DrawFilledWithAlpha(identity, quadArray, mColor, segmentAlpha);
        
        // Limpar o VertexArray
        delete quadArray;
    }
}

