//
// Created by Lucas N. Ferreira on 28/05/25.
//

#pragma once

#include <string>
#include "UIElement.h"

class UIImage : public UIElement
{
public:
    UIImage(class Game* game, const Vector2 &offset, float scale = 1.0f, float angle = 0.0f, int drawOrder = 100);
    UIImage(class Game* game, const std::string &imagePath, const Vector2 &offset, float scale = 1.0f, float angle = 0.0f, int drawOrder = 100);

    ~UIImage();

    void Draw(class Shader* shader) override;
    void SetColor(const Vector4 &color) { mColor = color; mUseColor = true; }

protected:
    class Texture* mTexture;
    Vector4 mColor;
    bool mUseColor;
};
