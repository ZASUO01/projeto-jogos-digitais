//
// Created by Lucas N. Ferreira on 03/08/23.
//

#pragma once
#include "Component.h"
#include "../Math.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexArray.h"
#include <vector>
#include <SDL.h>

class DrawComponent : public Component
{
public:
    // (Lower draw order corresponds with further back)
    DrawComponent(class Actor* owner, std::vector<Vector2> &vertices, int drawOrder = 100, Vector3 color = Vector3(1, 1, 1), bool filled = false);
    ~DrawComponent();

    virtual void Draw(Renderer* renderer);
    int GetDrawOrder() const { return mDrawOrder; }

    void SetVisible(bool visible) { mIsVisible = visible; }

protected:
    int mDrawOrder;
    bool mIsVisible;
    bool mIsFilled;
    Vector3 mColor;
    class VertexArray *mDrawArray;
};
