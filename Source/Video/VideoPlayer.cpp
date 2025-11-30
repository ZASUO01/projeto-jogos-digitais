#include "VideoPlayer.h"
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>

VideoPlayer::VideoPlayer()
    : mWindow(nullptr)
    , mSDLRenderer(nullptr)
    , mVideoTexture(nullptr)
    , mFormatContext(nullptr)
    , mCodecContext(nullptr)
    , mFrame(nullptr)
    , mFrameRGB(nullptr)
    , mSwsContext(nullptr)
    , mBuffer(nullptr)
    , mPacket(nullptr)
    , mVideoStreamIndex(-1)
    , mIsPlaying(false)
    , mLoop(false)
    , mVideoLoaded(false)
    , mFinished(false)
    , mFrameTime(0.0)
    , mStartTime(0.0)
    , mVideoClock(0.0)
    , mNextFrameTime(0.0)
    , mVideoWidth(0)
    , mVideoHeight(0)
{
}

VideoPlayer::~VideoPlayer()
{
    Shutdown();
}

bool VideoPlayer::PlayVideo(const std::string& filePath, SDL_Window* window, bool loop)
{
    if (mIsPlaying) {
        Stop();
    }

    mWindow = window;
    mLoop = loop;
    
    if (!mSDLRenderer) {
        mSDLRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!mSDLRenderer) {
            mSDLRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
            if (!mSDLRenderer) {
                SDL_Log("Failed to create SDL renderer: %s", SDL_GetError());
                return false;
            }
        }
    }
    
    if (!LoadVideo(filePath)) {
        return false;
    }

    mIsPlaying = true;
    mFinished = false;
    mStartTime = av_gettime() / 1000000.0;
    mVideoClock = 0.0;
    mNextFrameTime = 0.0;
    return true;
}

bool VideoPlayer::LoadVideo(const std::string& filePath)
{
    Cleanup();

    std::vector<std::string> pathsToTry;
    pathsToTry.push_back(filePath);
    pathsToTry.push_back("../" + filePath);
    pathsToTry.push_back("../../" + filePath);
    
    #if SDL_VERSION_ATLEAST(2, 0, 1)
    char* basePath = SDL_GetBasePath();
    if (basePath) {
        std::string basePathStr(basePath);
        SDL_free(basePath);
        pathsToTry.push_back(basePathStr + filePath);
        pathsToTry.push_back(basePathStr + "../" + filePath);
    }
    #endif
    
    std::string actualPath;
    bool found = false;
    
    for (const auto& path : pathsToTry) {
        std::ifstream file(path, std::ios::binary);
        if (file.good()) {
            file.close();
            if (avformat_open_input(&mFormatContext, path.c_str(), nullptr, nullptr) == 0) {
                actualPath = path;
                found = true;
                break;
            }
        }
    }
    
    if (!found) {
        SDL_Log("Could not open video file: %s", filePath.c_str());
        return false;
    }
    
    if (avformat_find_stream_info(mFormatContext, nullptr) < 0) {
        SDL_Log("Could not find stream info");
        Cleanup();
        return false;
    }

    mVideoStreamIndex = -1;
    for (unsigned int i = 0; i < mFormatContext->nb_streams; i++) {
        if (mFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            mVideoStreamIndex = i;
            break;
        }
    }

    if (mVideoStreamIndex == -1) {
        SDL_Log("Could not find video stream");
        Cleanup();
        return false;
    }

    AVCodecParameters* codecpar = mFormatContext->streams[mVideoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        SDL_Log("Codec not found");
        Cleanup();
        return false;
    }

    mCodecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(mCodecContext, codecpar) < 0) {
        SDL_Log("Could not copy codec context");
        Cleanup();
        return false;
    }

    if (avcodec_open2(mCodecContext, codec, nullptr) < 0) {
        SDL_Log("Could not open codec");
        Cleanup();
        return false;
    }

    mVideoWidth = mCodecContext->width;
    mVideoHeight = mCodecContext->height;

    AVRational frameRate = mFormatContext->streams[mVideoStreamIndex]->avg_frame_rate;
    if (frameRate.num > 0 && frameRate.den > 0) {
        mFrameTime = (double)frameRate.den / (double)frameRate.num;
    } else {
        mFrameTime = 1.0 / 30.0; // Default 30 FPS
    }

    // Alocar frames
    mFrame = av_frame_alloc();
    mFrameRGB = av_frame_alloc();
    if (!mFrame || !mFrameRGB) {
        SDL_Log("Could not allocate frames");
        Cleanup();
        return false;
    }

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, mVideoWidth, mVideoHeight, 1);
    mBuffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(mFrameRGB->data, mFrameRGB->linesize, mBuffer, AV_PIX_FMT_BGRA, mVideoWidth, mVideoHeight, 1);

    mSwsContext = sws_getContext(
        mVideoWidth, mVideoHeight, mCodecContext->pix_fmt,
        mVideoWidth, mVideoHeight, AV_PIX_FMT_BGRA,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
    
    if (!mSwsContext) {
        SDL_Log("Failed to create sws context");
        Cleanup();
        return false;
    }

    mVideoTexture = SDL_CreateTexture(mSDLRenderer, 
                                      SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      mVideoWidth, 
                                      mVideoHeight);
    
    if (!mVideoTexture) {
        SDL_Log("Failed to create video texture: %s", SDL_GetError());
        Cleanup();
        return false;
    }

    mPacket = av_packet_alloc();
    if (!mPacket) {
        SDL_Log("Failed to allocate packet");
        Cleanup();
        return false;
    }

    av_seek_frame(mFormatContext, mVideoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(mCodecContext);

    mVideoLoaded = true;
    mStartTime = av_gettime() / 1000000.0;
    mVideoClock = 0.0;
    mNextFrameTime = 0.0;

    return true;
}

void VideoPlayer::Stop()
{
    mIsPlaying = false;
}

void VideoPlayer::Render()
{
    RenderWithoutPresent();
    SDL_RenderPresent(mSDLRenderer);
}

void VideoPlayer::RenderWithoutPresent()
{
    if (!mIsPlaying || !mVideoTexture || !mSDLRenderer) {
        return;
    }

    SDL_SetRenderDrawColor(mSDLRenderer, 0, 0, 0, 255);
    SDL_RenderClear(mSDLRenderer);

    int windowWidth, windowHeight;
    SDL_GetWindowSize(mWindow, &windowWidth, &windowHeight);

    SDL_Rect destRect;
    float aspectRatio = (float)mVideoWidth / (float)mVideoHeight;
    float windowAspect = (float)windowWidth / (float)windowHeight;

    if (aspectRatio > windowAspect) {
        destRect.w = windowWidth;
        destRect.h = (int)(windowWidth / aspectRatio);
        destRect.x = 0;
        destRect.y = (windowHeight - destRect.h) / 2;
    } else {
        destRect.h = windowHeight;
        destRect.w = (int)(windowHeight * aspectRatio);
        destRect.x = (windowWidth - destRect.w) / 2;
        destRect.y = 0;
    }

    SDL_RenderCopy(mSDLRenderer, mVideoTexture, nullptr, &destRect);
}

void VideoPlayer::Cleanup()
{
    mIsPlaying = false;
    mVideoLoaded = false;

    if (mSwsContext) {
        sws_freeContext(mSwsContext);
        mSwsContext = nullptr;
    }

    if (mBuffer) {
        av_free(mBuffer);
        mBuffer = nullptr;
    }

    if (mFrameRGB) {
        av_frame_free(&mFrameRGB);
    }

    if (mFrame) {
        av_frame_free(&mFrame);
    }

    if (mPacket) {
        av_packet_free(&mPacket);
    }

    if (mCodecContext) {
        avcodec_free_context(&mCodecContext);
    }

    if (mFormatContext) {
        avformat_close_input(&mFormatContext);
    }

    if (mVideoTexture) {
        SDL_DestroyTexture(mVideoTexture);
        mVideoTexture = nullptr;
    }

    mVideoStreamIndex = -1;
}

void VideoPlayer::Shutdown()
{
    Stop();
    Cleanup();

    if (mSDLRenderer) {
        SDL_DestroyRenderer(mSDLRenderer);
        mSDLRenderer = nullptr;
    }

    mWindow = nullptr;
}

bool VideoPlayer::Run()
{
    if (!mIsPlaying) {
        return false;
    }

    SDL_Event event;
    bool running = true;

    while (running && mIsPlaying) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || 
                    event.key.keysym.sym == SDLK_RETURN || 
                    event.key.keysym.sym == SDLK_SPACE) {
                    running = false;
                    break;
                }
            }
        }

        if (!running) {
            break;
        }

        Update();
        Render();

        SDL_Delay(1);
    }

    Stop();
    return !running;
}

void VideoPlayer::Update()
{
    if (!mIsPlaying || !mVideoLoaded) {
        return;
    }

    double currentTime = av_gettime() / 1000000.0 - mStartTime;
    
    if (currentTime < mNextFrameTime) {
        return;
    }

    int ret = av_read_frame(mFormatContext, mPacket);
    
    if (ret < 0) {
        if (ret == AVERROR_EOF) {
            if (mLoop) {
                av_seek_frame(mFormatContext, mVideoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
                avcodec_flush_buffers(mCodecContext);
                mVideoClock = 0.0;
                mNextFrameTime = 0.0;
                mStartTime = av_gettime() / 1000000.0;
                return;
            } else {
                mFinished = true;
                Stop();
                return;
            }
        } else {
            return;
        }
    }

    if (mPacket->stream_index == mVideoStreamIndex) {
        if (avcodec_send_packet(mCodecContext, mPacket) == 0) {
            int decodeRet = avcodec_receive_frame(mCodecContext, mFrame);
            if (decodeRet == 0) {
                sws_scale(mSwsContext,
                         mFrame->data, mFrame->linesize, 0, mVideoHeight,
                         mFrameRGB->data, mFrameRGB->linesize);

                if (mFrame->pts != AV_NOPTS_VALUE) {
                    AVRational timeBase = mFormatContext->streams[mVideoStreamIndex]->time_base;
                    mVideoClock = mFrame->pts * av_q2d(timeBase);
                } else {
                    mVideoClock = currentTime;
                }

                if (mVideoTexture) {
                    void* pixels;
                    int pitch;
                    SDL_LockTexture(mVideoTexture, nullptr, &pixels, &pitch);
                    
                    uint8_t* src = mFrameRGB->data[0];
                    uint8_t* dst = (uint8_t*)pixels;
                    int srcPitch = mFrameRGB->linesize[0];
                    int bytesPerRow = mVideoWidth * 4;
                    int copyWidth = std::min(bytesPerRow, std::min(pitch, srcPitch));
                    
                    for (int y = 0; y < mVideoHeight; y++) {
                        memcpy(dst, src, copyWidth);
                        src += srcPitch;
                        dst += pitch;
                    }
                    
                    SDL_UnlockTexture(mVideoTexture);
                }

                mNextFrameTime = currentTime + mFrameTime;
            }
        }
    }
    
    av_packet_unref(mPacket);
}
