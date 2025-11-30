#pragma once
#include <SDL.h>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

class VideoPlayer
{
public:
    VideoPlayer();
    ~VideoPlayer();

    bool PlayVideo(const std::string& filePath, SDL_Window* window, bool loop = false);
    bool Run();
    void Update();
    void Render();
    void RenderWithoutPresent();
    bool IsPlaying() const { return mIsPlaying; }
    bool HasFinished() const { return mFinished; }
    double GetVideoClock() const { return mVideoClock; }
    SDL_Renderer* GetRenderer() const { return mSDLRenderer; }
    void Stop();
    void Shutdown();

private:
    bool LoadVideo(const std::string& filePath);
    void Cleanup();

    SDL_Window* mWindow;
    SDL_Renderer* mSDLRenderer;
    SDL_Texture* mVideoTexture;
    AVFormatContext* mFormatContext;
    AVCodecContext* mCodecContext;
    AVFrame* mFrame;
    AVFrame* mFrameRGB;
    struct SwsContext* mSwsContext;
    uint8_t* mBuffer;
    AVPacket* mPacket;
    int mVideoStreamIndex;
    bool mIsPlaying;
    bool mLoop;
    bool mVideoLoaded;
    bool mFinished;
    double mFrameTime;
    double mStartTime;
    double mVideoClock;
    double mNextFrameTime;
    int mVideoWidth;
    int mVideoHeight;
};
