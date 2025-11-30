//
// Created for grid drawing
//

#pragma once
#include "DrawComponent.h"
#include "../Math.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexArray.h"
#include <vector>

class GridDrawComponent : public DrawComponent
{
public:
    GridDrawComponent(class Actor* owner, float width, float height, float cellSize, int drawOrder = 0, Vector3 color = Vector3(1, 1, 1));
    ~GridDrawComponent();

    virtual void Draw(Renderer* renderer) override;

private:
    float mWidth;
    float mHeight;
    float mCellSize;
    class VertexArray* mGridArray;
};

