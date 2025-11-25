//
// Created by Lucas N. Ferreira on 10/09/23.
//

#include "CircleColliderComponent.h"
#include "../Actors/Actor.h"


CircleColliderComponent::CircleColliderComponent(class Actor* owner, const float radius, const int updateOrder)
        :Component(owner, updateOrder)
        ,mRadius(radius)
{
    std::vector<float> vertices = CreateCircleVertices(radius);
    std::vector<unsigned int> indices;

    unsigned int size = vertices.size();
    unsigned int indicesSize = 10;
    for (unsigned int i = 0; i < indicesSize; i++) {
        indices.emplace_back(i);
    }

    mDrawArray = new VertexArray(vertices.data(), size, indices.data(), indicesSize);
}

CircleColliderComponent::~CircleColliderComponent()
{
    delete mDrawArray;
    mDrawArray = nullptr;
}

bool CircleColliderComponent::Intersect(const CircleColliderComponent& c) const
{
    float radiusSum = mRadius + c.mRadius;
    float distanceX = Math::Abs(mOwner->GetPosition().x - c.mOwner->GetPosition().x);
    float distanceY = Math::Abs(mOwner->GetPosition().y - c.mOwner->GetPosition().y);

    return (((distanceX * distanceX) + (distanceY * distanceY)) <= (radiusSum * radiusSum));
}

void CircleColliderComponent::DebugDraw(Renderer *renderer)
{
    renderer->Draw(
            mOwner->GetModelMatrix(),
            mDrawArray,
            Vector3(0, 1, 0)
    );
}

std::vector<float> CircleColliderComponent::CreateCircleVertices(float radius) {
    std::vector<float> vertices;

    constexpr int numSides = 10;
    constexpr float step = Math::TwoPi / numSides;
    float angle = 0.0f;

    vertices.emplace_back(radius);
    vertices.emplace_back(0);

    for (int i = 1; i < numSides; i++) {
        angle += step;
        vertices.emplace_back(radius * cos(angle));
        vertices.emplace_back(radius * sin(angle));
    }

    return vertices;
}