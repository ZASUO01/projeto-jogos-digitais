//
// Created by Lucas N. Ferreira on 06/11/25.
//

#pragma once

#include "UIScreen.h"

class EndGame : public UIScreen
{
public:
    EndGame(class Game* game, const std::string& fontName);

    void HandleKeyPress(int key) override;

private:
    void UpdateArrows();

    class UITriangle* mLeftArrow;
    class UITriangle* mRightArrow;
};
