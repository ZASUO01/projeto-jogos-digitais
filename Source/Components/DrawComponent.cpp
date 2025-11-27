//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "DrawComponent.h"
#include "CircleColliderComponent.h"
#include "../Game.h"

DrawComponent::DrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder, Vector3 color, bool filled)
    :Component(owner)
    ,mDrawOrder(drawOrder)
    ,mIsVisible(true)
    ,mIsFilled(filled)
    ,mColor(color)
{
    mOwner->GetGame()->AddDrawable(this);

    std::vector<float> floatVertices;
    std::vector<unsigned int> indices;
    unsigned int size = vertices.size();
    for (unsigned int i = 0; i < size; i++) {
        floatVertices.emplace_back(vertices[i].x);
        floatVertices.emplace_back(vertices[i].y);
        indices.emplace_back(i);
    }

    mDrawArray = new VertexArray(floatVertices.data(), size * 2, indices.data(), size);
}

DrawComponent::~DrawComponent()
{
    mOwner->GetGame()->RemoveDrawable(this);
    delete mDrawArray;
    mDrawArray = nullptr;
}

void DrawComponent::Draw(Renderer *renderer)
{
    if (mOwner->GetState() == ActorState::Active) {
        if (mIsFilled) {
            renderer->DrawFilled(
                mOwner->GetModelMatrix(),
                mDrawArray,
                mColor
            );
        } else {
            renderer->Draw(
                mOwner->GetModelMatrix(),
                mDrawArray,
                mColor
            );
        }
    }
}
