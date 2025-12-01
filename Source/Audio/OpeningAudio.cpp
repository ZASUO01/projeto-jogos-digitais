#include "OpeningAudio.h"
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <vector>

OpeningAudio::OpeningAudio()
    : mBeginChunk(nullptr)
    , mAberturaChunk(nullptr)
    , mLoopChunk(nullptr)
    , mShootChunk(nullptr)
    , mBeginChannel(-1)
    , mAberturaChannel(-1)
    , mLoopChannel(-1)
    , mBeginLoop(false)
    , mAberturaLoop(false)
    , mLoopLoop(false)
    , mBeginPlaying(false)
    , mAberturaPlaying(false)
    , mLoopPlaying(false)
    , mInitialized(false)
{
}

OpeningAudio::~OpeningAudio()
{
    Shutdown();
}

bool OpeningAudio::Initialize()
{
    if (mInitialized) {
        return true;
    }

    // Inicializar SDL Mixer (WAV não precisa de codecs especiais)

    // Inicializar SDL Mixer se ainda não foi inicializado
    int audio_rate = 44100;
    Uint16 audio_format = MIX_DEFAULT_FORMAT;
    int audio_channels = 2;
    int audio_buffers = 2048;
    
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
        SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    
    // Verificar quais codecs estão disponíveis
    int numTimesOpened = Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    SDL_Log("SDL_mixer initialized: %d Hz, %d channels", audio_rate, audio_channels);

    // Tentar carregar os arquivos de áudio WAV (usando o mesmo padrão dos vídeos)
    std::vector<std::string> pathsToTry;
    std::string beginPath, aberturaPath, loopPath, shootPath;
    bool foundBegin = false, foundAbertura = false, foundLoop = false, foundShoot = false;

    // Tentar begin.wav
    pathsToTry.push_back("Opening/begin.wav");
    pathsToTry.push_back("../Opening/begin.wav");
    pathsToTry.push_back("../../Opening/begin.wav");
    
    #if SDL_VERSION_ATLEAST(2, 0, 1)
    char* basePath = SDL_GetBasePath();
    if (basePath) {
        std::string basePathStr(basePath);
        SDL_free(basePath);
        pathsToTry.push_back(basePathStr + "Opening/begin.wav");
        pathsToTry.push_back(basePathStr + "../Opening/begin.wav");
    }
    #endif

    for (const auto& path : pathsToTry) {
        std::ifstream file(path, std::ios::binary);
        if (file.good()) {
            file.close();
            beginPath = path;
            foundBegin = true;
            break;
        }
    }

    // Tentar abertura.wav
    pathsToTry.clear();
    pathsToTry.push_back("Opening/abertura.wav");
    pathsToTry.push_back("../Opening/abertura.wav");
    pathsToTry.push_back("../../Opening/abertura.wav");
    
    #if SDL_VERSION_ATLEAST(2, 0, 1)
    basePath = SDL_GetBasePath();
    if (basePath) {
        std::string basePathStr(basePath);
        SDL_free(basePath);
        pathsToTry.push_back(basePathStr + "Opening/abertura.wav");
        pathsToTry.push_back(basePathStr + "../Opening/abertura.wav");
    }
    #endif

    for (const auto& path : pathsToTry) {
        std::ifstream file(path, std::ios::binary);
        if (file.good()) {
            file.close();
            aberturaPath = path;
            foundAbertura = true;
            break;
        }
    }

    // Tentar loop.wav
    pathsToTry.clear();
    pathsToTry.push_back("Opening/loop.wav");
    pathsToTry.push_back("../Opening/loop.wav");
    pathsToTry.push_back("../../Opening/loop.wav");
    
    #if SDL_VERSION_ATLEAST(2, 0, 1)
    basePath = SDL_GetBasePath();
    if (basePath) {
        std::string basePathStr(basePath);
        SDL_free(basePath);
        pathsToTry.push_back(basePathStr + "Opening/loop.wav");
        pathsToTry.push_back(basePathStr + "../Opening/loop.wav");
    }
    #endif

    for (const auto& path : pathsToTry) {
        std::ifstream file(path, std::ios::binary);
        if (file.good()) {
            file.close();
            loopPath = path;
            foundLoop = true;
            break;
        }
    }

    // Carregar os arquivos WAV usando Mix_LoadWAV (seguindo padrão do projeto de referência)
    if (foundBegin) {
        SDL_Log("Attempting to load begin.wav from: %s", beginPath.c_str());
        mBeginChunk = Mix_LoadWAV(beginPath.c_str());
        if (!mBeginChunk) {
            SDL_Log("Failed to load begin.wav: %s", Mix_GetError());
        } else {
            SDL_Log("Successfully loaded begin.wav");
        }
    } else {
        SDL_Log("Could not find begin.wav in any of the searched paths");
    }

    if (foundAbertura) {
        SDL_Log("Attempting to load abertura.wav from: %s", aberturaPath.c_str());
        mAberturaChunk = Mix_LoadWAV(aberturaPath.c_str());
        if (!mAberturaChunk) {
            SDL_Log("Failed to load abertura.wav: %s", Mix_GetError());
        } else {
            SDL_Log("Successfully loaded abertura.wav");
        }
    } else {
        SDL_Log("Could not find abertura.wav in any of the searched paths");
    }

    if (foundLoop) {
        SDL_Log("Attempting to load loop.wav from: %s", loopPath.c_str());
        mLoopChunk = Mix_LoadWAV(loopPath.c_str());
        if (!mLoopChunk) {
            SDL_Log("Failed to load loop.wav: %s", Mix_GetError());
        } else {
            SDL_Log("Successfully loaded loop.wav");
        }
    } else {
        SDL_Log("Could not find loop.wav in any of the searched paths");
    }

    // Tentar Shoot.wav (som de tiro)
    pathsToTry.clear();
    pathsToTry.push_back("Opening/Shoot.wav");
    pathsToTry.push_back("../Opening/Shoot.wav");
    pathsToTry.push_back("../../Opening/Shoot.wav");
    
    #if SDL_VERSION_ATLEAST(2, 0, 1)
    basePath = SDL_GetBasePath();
    if (basePath) {
        std::string basePathStr(basePath);
        SDL_free(basePath);
        pathsToTry.push_back(basePathStr + "Opening/Shoot.wav");
        pathsToTry.push_back(basePathStr + "../Opening/Shoot.wav");
    }
    #endif

    for (const auto& path : pathsToTry) {
        std::ifstream file(path, std::ios::binary);
        if (file.good()) {
            file.close();
            shootPath = path;
            foundShoot = true;
            break;
        }
    }

    if (foundShoot) {
        SDL_Log("Attempting to load Shoot.wav from: %s", shootPath.c_str());
        mShootChunk = Mix_LoadWAV(shootPath.c_str());
        if (!mShootChunk) {
            SDL_Log("Failed to load Shoot.wav: %s", Mix_GetError());
        } else {
            SDL_Log("Successfully loaded Shoot.wav");
        }
    } else {
        SDL_Log("Could not find Shoot.wav in any of the searched paths");
    }

    mInitialized = true;
    return true;
}

void OpeningAudio::Shutdown()
{
    Stop();

    if (mBeginChunk) {
        Mix_FreeChunk(mBeginChunk);
        mBeginChunk = nullptr;
    }

    if (mAberturaChunk) {
        Mix_FreeChunk(mAberturaChunk);
        mAberturaChunk = nullptr;
    }

    if (mLoopChunk) {
        Mix_FreeChunk(mLoopChunk);
        mLoopChunk = nullptr;
    }

    if (mShootChunk) {
        Mix_FreeChunk(mShootChunk);
        mShootChunk = nullptr;
    }

    if (mInitialized) {
        Mix_CloseAudio();
        mInitialized = false;
    }
}

bool OpeningAudio::PlayBegin(bool loop)
{
    if (!mInitialized || !mBeginChunk) {
        return false;
    }

    Stop(); // Parar todos os outros áudios

    mBeginLoop = loop;
    int loops = loop ? -1 : 0;
    
    mBeginChannel = Mix_PlayChannel(-1, mBeginChunk, loops);
    if (mBeginChannel == -1) {
        SDL_Log("Failed to play begin.wav: %s", Mix_GetError());
        return false;
    }

    mBeginPlaying = true;
    mAberturaPlaying = false;
    mLoopPlaying = false;

    return true;
}

bool OpeningAudio::PlayAbertura(bool loop)
{
    if (!mInitialized || !mAberturaChunk) {
        return false;
    }

    Stop(); // Parar todos os outros áudios

    mAberturaLoop = loop;
    int loops = loop ? -1 : 0;
    
    mAberturaChannel = Mix_PlayChannel(-1, mAberturaChunk, loops);
    if (mAberturaChannel == -1) {
        SDL_Log("Failed to play abertura.wav: %s", Mix_GetError());
        return false;
    }

    mBeginPlaying = false;
    mAberturaPlaying = true;
    mLoopPlaying = false;

    return true;
}

bool OpeningAudio::PlayLoop(bool loop)
{
    if (!mInitialized || !mLoopChunk) {
        return false;
    }

    Stop(); // Parar todos os outros áudios

    mLoopLoop = loop;
    int loops = loop ? -1 : 0;
    
    mLoopChannel = Mix_PlayChannel(-1, mLoopChunk, loops);
    if (mLoopChannel == -1) {
        SDL_Log("Failed to play loop.wav: %s", Mix_GetError());
        return false;
    }

    mBeginPlaying = false;
    mAberturaPlaying = false;
    mLoopPlaying = true;

    return true;
}

bool OpeningAudio::PlayLoopBackground(bool loop, int volume)
{
    if (!mInitialized || !mLoopChunk) {
        return false;
    }

    // Não parar outros áudios se já estiver tocando o loop
    if (!mLoopPlaying) {
        mLoopLoop = loop;
        int loops = loop ? -1 : 0;
        
        mLoopChannel = Mix_PlayChannel(-1, mLoopChunk, loops);
        if (mLoopChannel == -1) {
            SDL_Log("Failed to play loop.wav as background: %s", Mix_GetError());
            return false;
        }

        // Ajustar volume para ser baixo mas perceptível (volume de 0-128, usar ~30 para ~23% do volume)
        Mix_Volume(mLoopChannel, volume);
        
        mLoopPlaying = true;
    } else {
        // Se já está tocando, apenas ajustar o volume
        Mix_Volume(mLoopChannel, volume);
    }

    return true;
}

void OpeningAudio::Stop()
{
    if (mBeginChannel != -1) {
        Mix_HaltChannel(mBeginChannel);
        mBeginChannel = -1;
    }
    if (mAberturaChannel != -1) {
        Mix_HaltChannel(mAberturaChannel);
        mAberturaChannel = -1;
    }
    if (mLoopChannel != -1) {
        Mix_HaltChannel(mLoopChannel);
        mLoopChannel = -1;
    }
    
    mBeginPlaying = false;
    mAberturaPlaying = false;
    mLoopPlaying = false;
    mBeginLoop = false;
    mAberturaLoop = false;
    mLoopLoop = false;
}

void OpeningAudio::StopBegin()
{
    if (mBeginChannel != -1) {
        Mix_HaltChannel(mBeginChannel);
        mBeginChannel = -1;
        mBeginPlaying = false;
        mBeginLoop = false;
    }
}

void OpeningAudio::StopAbertura()
{
    if (mAberturaChannel != -1) {
        Mix_HaltChannel(mAberturaChannel);
        mAberturaChannel = -1;
        mAberturaPlaying = false;
        mAberturaLoop = false;
    }
}

void OpeningAudio::StopLoop()
{
    if (mLoopChannel != -1) {
        Mix_HaltChannel(mLoopChannel);
        mLoopChannel = -1;
        mLoopPlaying = false;
        mLoopLoop = false;
    }
}

bool OpeningAudio::IsPlaying() const
{
    return IsBeginPlaying() || IsAberturaPlaying() || IsLoopPlaying();
}

bool OpeningAudio::IsBeginPlaying() const
{
    if (mBeginChannel == -1) {
        return false;
    }
    return Mix_Playing(mBeginChannel) != 0;
}

bool OpeningAudio::IsAberturaPlaying() const
{
    if (mAberturaChannel == -1) {
        return false;
    }
    return Mix_Playing(mAberturaChannel) != 0;
}

bool OpeningAudio::IsLoopPlaying() const
{
    if (mLoopChannel == -1) {
        return false;
    }
    return Mix_Playing(mLoopChannel) != 0;
}

void OpeningAudio::Update()
{
    // Otimização: só verificar canais que estão realmente ativos e precisam de loop
    // Verificar begin apenas se estiver tocando E precisar fazer loop
    if (mBeginPlaying && mBeginLoop && mBeginChannel != -1) {
        if (!Mix_Playing(mBeginChannel)) {
            // Áudio terminou, reiniciar loop
            mBeginChannel = Mix_PlayChannel(-1, mBeginChunk, -1);
            if (mBeginChannel == -1) {
                mBeginPlaying = false;
            }
        }
    }

    // Verificar abertura apenas se estiver tocando E precisar fazer loop
    // (abertura normalmente não faz loop, então esta verificação raramente será necessária)
    if (mAberturaPlaying && mAberturaLoop && mAberturaChannel != -1) {
        if (!Mix_Playing(mAberturaChannel)) {
            // Áudio terminou, reiniciar loop
            mAberturaChannel = Mix_PlayChannel(-1, mAberturaChunk, -1);
            if (mAberturaChannel == -1) {
                mAberturaPlaying = false;
            }
        }
    }

    // Verificar loop apenas se estiver tocando E precisar fazer loop
    if (mLoopPlaying && mLoopLoop && mLoopChannel != -1) {
        if (!Mix_Playing(mLoopChannel)) {
            // Áudio terminou, reiniciar loop
            mLoopChannel = Mix_PlayChannel(-1, mLoopChunk, -1);
            if (mLoopChannel == -1) {
                mLoopPlaying = false;
            }
        }
    }
}

bool OpeningAudio::PlayShoot()
{
    if (!mInitialized || !mShootChunk) {
        return false;
    }

    // Tocar o som de tiro em um canal separado (não interfere com outros sons)
    // Volume padrão (128) para ser audível
    int channel = Mix_PlayChannel(-1, mShootChunk, 0);
    if (channel == -1) {
        SDL_Log("Failed to play Shoot.wav: %s", Mix_GetError());
        return false;
    }

    return true;
}

