#include "AudioPlayer.h"
#include <SDL.h>
#include <iostream>
#include <cmath>

AudioPlayer* AudioPlayer::sCurrentInstance = nullptr;

AudioPlayer::AudioPlayer()
    : mChunk(nullptr)
    , mChannel(-1)
    , mIsPlaying(false)
    , mIsPaused(false)
    , mFinished(false)
    , mLoaded(false)
    , mLooping(false)
    , mVolume(128)
    , mDuration(0.0)
    , mStartTime(0)
{
}

AudioPlayer::~AudioPlayer()
{
    Unload();
}

void AudioPlayer::ChannelFinishedCallback(int channel)
{
    if (sCurrentInstance && sCurrentInstance->mChannel == channel)
    {
        sCurrentInstance->mFinished = true;
        sCurrentInstance->mIsPlaying = false;
        
        // Se estiver em loop, reiniciar
        if (sCurrentInstance->mLooping)
        {
            Mix_PlayChannel(channel, sCurrentInstance->mChunk, -1);
            sCurrentInstance->mIsPlaying = true;
            sCurrentInstance->mFinished = false;
            sCurrentInstance->mStartTime = SDL_GetTicks();
        }
    }
}

bool AudioPlayer::Load(const std::string& fileName)
{
    Unload();
    
    mChunk = Mix_LoadWAV(fileName.c_str());
    if (!mChunk)
    {
        SDL_Log("Erro ao carregar arquivo de áudio: %s - %s", fileName.c_str(), Mix_GetError());
        return false;
    }
    
    // Calcular duração aproximada
    // WAV: samples / sample_rate
    // Para um chunk WAV, podemos estimar baseado no tamanho
    // Mas para precisão, precisaríamos decodificar o arquivo
    // Por enquanto, vamos usar uma estimativa baseada no tamanho do chunk
    // Isso não é preciso, mas funciona para controle de loop
    mDuration = 0.0; // Será calculado quando necessário
    
    mFileName = fileName;
    mLoaded = true;
    mFinished = false;
    mIsPlaying = false;
    mIsPaused = false;
    
    // Registrar callback
    sCurrentInstance = this;
    Mix_ChannelFinished(ChannelFinishedCallback);
    
    return true;
}

void AudioPlayer::Unload()
{
    Stop();
    
    if (mChunk)
    {
        Mix_FreeChunk(mChunk);
        mChunk = nullptr;
    }
    
    mLoaded = false;
    mFinished = false;
    mIsPlaying = false;
    mIsPaused = false;
    mChannel = -1;
}

void AudioPlayer::Play(bool loop)
{
    if (!mLoaded || !mChunk)
        return;
    
    Stop();
    
    mLooping = loop;
    int loops = loop ? -1 : 0;
    
    mChannel = Mix_PlayChannel(-1, mChunk, loops);
    if (mChannel == -1)
    {
        SDL_Log("Erro ao tocar áudio: %s", Mix_GetError());
        mIsPlaying = false;
        return;
    }
    
    Mix_Volume(mChannel, mVolume);
    mIsPlaying = true;
    mIsPaused = false;
    mFinished = false;
    mStartTime = SDL_GetTicks();
}

void AudioPlayer::Stop()
{
    if (mChannel != -1 && mIsPlaying)
    {
        Mix_HaltChannel(mChannel);
    }
    
    mChannel = -1;
    mIsPlaying = false;
    mIsPaused = false;
    mFinished = false;
}

void AudioPlayer::Pause()
{
    if (mChannel != -1 && mIsPlaying && !mIsPaused)
    {
        Mix_Pause(mChannel);
        mIsPaused = true;
    }
}

void AudioPlayer::Resume()
{
    if (mChannel != -1 && mIsPaused)
    {
        Mix_Resume(mChannel);
        mIsPaused = false;
    }
}

bool AudioPlayer::IsPlaying() const
{
    if (mChannel == -1)
        return false;
    
    return mIsPlaying && !mIsPaused && Mix_Playing(mChannel);
}

bool AudioPlayer::IsPaused() const
{
    return mIsPaused;
}

void AudioPlayer::SetVolume(int volume)
{
    mVolume = volume;
    if (mVolume < 0) mVolume = 0;
    if (mVolume > 128) mVolume = 128;
    
    if (mChannel != -1)
    {
        Mix_Volume(mChannel, mVolume);
    }
}

double AudioPlayer::GetCurrentTime() const
{
    if (!mIsPlaying || mChannel == -1)
        return 0.0;
    
    // Para WAV chunks, não temos acesso direto ao tempo atual
    // Podemos estimar baseado no tempo decorrido desde o início
    Uint32 elapsed = SDL_GetTicks() - mStartTime;
    return elapsed / 1000.0;
}

