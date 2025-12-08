#pragma once

#include "UIScreen.h"

class Renderer;

class OpeningScreen : public UIScreen
{
public:
    OpeningScreen(class Game* game);
    ~OpeningScreen();

    void Update(float deltaTime) override;
    void HandleKeyPress(int key) override;
    void Draw(class Renderer* renderer);

private:
    class VideoPlayer* mVideoPlayer;
    class AudioPlayer* mAudioPlayer;
    bool mIsLooping;
    bool mHasPlayedOnce;
    class VertexArray* mVideoVerts;
};

