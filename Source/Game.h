// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <SDL.h>
#include <vector>

#include "Actors/Ship.h"
#include "Actors/Actor.h"
#include "Renderer/Renderer.h"

// Forward declaration
class VideoPlayer;
class MenuHUD;
class OpeningAudio;

class Game{
public:
    Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    void InitializeActors();
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);

    class Renderer* GetRenderer() { return mRenderer; }

    static const int WINDOW_WIDTH = 1280;
    static const int WINDOW_HEIGHT = 720;
    static const int RENDER_WIDTH = 1280;
    static const int RENDER_HEIGHT = 720;
    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    std::vector<class DrawComponent*>& GetDrawables() { return mDrawables; }

    Ship *GetShip() const {return mShip; }
    Ship *GetShip1() const {return mShip1; }
    Ship *GetShip2() const {return mShip2; }
    OpeningAudio* GetOpeningAudio() { return mOpeningAudio; }
    
    // Modo debug: quando ativo, pula direto para a terceira fase (EntranceLoop)
    static const bool DEBUG_MODE = true; // Altere para false para desativar
    
private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    void RemoveActorFromVector(std::vector< Actor*> &actors,  Actor *actor);
    void CheckLaserCollisions();

    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;
    std::vector<class DrawComponent*> mDrawables;
    
    SDL_Window* mWindow;
    class Renderer* mRenderer;
    Uint32 mTicksCount;
    bool mIsRunning;
    bool mIsDebugging;
    bool mUpdatingActors;

    // Video intro
    VideoPlayer* mVideoPlayer;
    bool mShowingVideo;
    enum class VideoState {
        Begin,           // begin.mp4 em loop
        Abertura,        // abertura.mp4 (sem loop)
        EntranceLoop     // entrance_loop.mp4 em loop
    };
    VideoState mVideoState;
    double mAberturaStartTime;  // Tempo quando abertura.mp4 começou
    double mAberturaAudioStartTime;  // Tempo quando o áudio da abertura deve começar
    bool mAberturaAudioPending;  // Se o áudio da abertura está aguardando para começar

    // Menu HUD
    MenuHUD* mMenuHUD;

    // Opening Audio
    OpeningAudio* mOpeningAudio;

    // Game-specific
    Ship* mShip;
    Ship* mShip1;
    Ship* mShip2;
};
