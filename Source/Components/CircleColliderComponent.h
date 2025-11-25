//
// Created by Lucas N. Ferreira on 10/09/23.
//

#pragma once
#include "Component.h"
#include "../Math.h"
#include <vector>

class CircleColliderComponent : public Component
{
public:
    CircleColliderComponent(class Actor* owner, float radius, int updateOrder = 10);
    ~CircleColliderComponent();

    // Drawing for debug purposes
    void DebugDraw(class Renderer* renderer) override;

    // Setters and getters
    void SetRadius(float radius) { mRadius = radius; }
    float GetRadius() const { return mRadius; }

    // Check intersection between this circle and another
    bool Intersect(const CircleColliderComponent& b) const;

private:
    std::vector<float> CreateCircleVertices(float radius);
    float mRadius;
    class VertexArray *mDrawArray;
};

