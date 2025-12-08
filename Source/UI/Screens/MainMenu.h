//
// Created by Lucas N. Ferreira on 06/11/25.
//

#pragma once

#include "UIScreen.h"

class MainMenu : public UIScreen
{
public:
    MainMenu(class Game* game, const std::string& fontName);
    ~MainMenu();

    void Update(float deltaTime) override;
    void HandleKeyPress(int key) override;

private:
    void UpdateArrows();
    
    class VideoPlayer* mVideoPlayer;
    class UIVideo* mVideoImage;
    class UITriangle* mLeftArrow;
    class UITriangle* mRightArrow;
};