#pragma once
#include "DrawComponent.h"
#include "../Math.h"
#include <vector>

struct TrailPoint
{
    Vector2 position;
    float age; // Tempo desde que foi criado
    float maxAge; // Tempo máximo de vida
};

class TrailComponent : public DrawComponent
{
public:
    TrailComponent(class Actor* owner, Vector3 color, float trailLength = 0.8f, float updateInterval = 0.02f);
    ~TrailComponent();
    
    void Update(float deltaTime) override;
    void Draw(class Renderer* renderer) override;
    
    void SetColor(Vector3 color) { mColor = color; }
    void SetTrailLength(float length) { mTrailLength = length; }
    
private:
    void AddTrailPoint(const Vector2& position);
    void UpdateTrailPoints(float deltaTime);
    void RemoveOldPoints();
    
    std::vector<TrailPoint> mTrailPoints;
    float mTrailLength; // Duração do rastro em segundos
    float mUpdateInterval; // Intervalo entre adicionar novos pontos
    float mTimeSinceLastUpdate;
    static const int MAX_TRAIL_POINTS = 50; // Limite de pontos no rastro
};

