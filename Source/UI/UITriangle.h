#pragma once

#include "UIElement.h"
#include "../Math.h"

class UITriangle : public UIElement
{
public:
    UITriangle(class Game* game, const Vector2& offset, float size, float angle, int drawOrder = 100);
    ~UITriangle();

    void Draw(class Shader* shader) override;
    void SetColor(const Vector4& color) { mColor = color; }

private:
    float mSize;
    Vector4 mColor;
    class VertexArray* mTriangleVerts;
};

