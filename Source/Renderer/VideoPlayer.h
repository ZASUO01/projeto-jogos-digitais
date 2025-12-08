#pragma once

#include <string>
#include <GL/glew.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VideoPlayer
{
public:
    VideoPlayer();
    ~VideoPlayer();

    bool Load(const std::string& fileName);
    void Unload();
    
    bool Update(float deltaTime);
    void Render(class Renderer* renderer);
    
    bool IsFinished() const { return mFinished; }
    void SeekToTime(double timeInSeconds);
    void ResetFinished() { mFinished = false; }
    double GetCurrentTime() const { return mCurrentTime; }
    double GetDuration() const { return mDuration; }
    
    unsigned int GetTextureID() const { return mTextureID; }
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }

private:
    bool DecodeFrame();
    void UpdateTexture();
    
    AVFormatContext* mFormatContext;
    AVCodecContext* mCodecContext;
    AVFrame* mFrame;
    AVFrame* mFrameRGB;
    AVPacket* mPacket;
    struct SwsContext* mSwsContext;
    
    int mVideoStreamIndex;
    unsigned int mTextureID;
    int mWidth;
    int mHeight;
    
    double mCurrentTime;
    double mDuration;
    double mFrameTime;
    double mAccumulatedTime;
    
    bool mFinished;
    bool mLoaded;
    
    uint8_t* mFrameBuffer;
};

