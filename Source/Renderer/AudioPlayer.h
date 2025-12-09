#pragma once

#include <string>
#include <SDL_mixer.h>

class AudioPlayer
{
public:
    AudioPlayer();
    ~AudioPlayer();

    bool Load(const std::string& fileName);
    void Unload();
    
    void Play(bool loop = false);
    void Stop();
    void Pause();
    void Resume();
    
    bool IsPlaying() const;
    bool IsPaused() const;
    bool IsFinished() const { return mFinished; }
    void ResetFinished() { mFinished = false; }
    
    void SetVolume(int volume); // 0-128
    int GetVolume() const { return mVolume; }
    
    double GetDuration() const { return mDuration; }
    double GetCurrentTime() const;

private:
    static void ChannelFinishedCallback(int channel);
    static AudioPlayer* sCurrentInstance;
    
    Mix_Chunk* mChunk;
    int mChannel;
    bool mIsPlaying;
    bool mIsPaused;
    bool mFinished;
    bool mLoaded;
    bool mLooping;
    int mVolume;
    double mDuration;
    Uint32 mStartTime;
    std::string mFileName;
};

