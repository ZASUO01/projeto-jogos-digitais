#pragma once

#include "UIImage.h"

class UIVideo : public UIImage
{
public:
    UIVideo(class Game* game, class VideoPlayer* videoPlayer, const Vector2& offset, float scale = 1.0f, float angle = 0.0f, int drawOrder = 100);
    
    void Draw(class Shader* shader) override;
    
private:
    class VideoPlayer* mVideoPlayer;
    unsigned int mVideoTextureID;
};

