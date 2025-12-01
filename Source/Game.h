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

// Forward declarations
class VideoPlayer;
class MenuHUD;
class OpeningAudio;
class Actor;
class DrawComponent;
class LaserBeam;
class LaserBeamComponent;

// ============================================================================
// Game Scene Enumeration
// ============================================================================
enum class GameScene
{
    Opening,    // Tela de abertura com vídeo introdutório
    MainMenu,   // Menu principal
    Gameplay    // Jogo em si
};

// ============================================================================
// Video State Enumeration
// ============================================================================
enum class VideoState
{
    Begin,          // begin.mp4 em loop
    Abertura,       // abertura.mp4 (sem loop)
    EntranceLoop    // entrance_loop.mp4 em loop
};

// ============================================================================
// Game Class
// ============================================================================
class Game
{
public:
    // ========================================================================
    // Constants
    // ========================================================================
    static const int WINDOW_WIDTH = 1280;
    static const int WINDOW_HEIGHT = 720;
    static const int RENDER_WIDTH = 1280;
    static const int RENDER_HEIGHT = 720;
    static const bool DEBUG_MODE = true;  // Pula direto para EntranceLoop quando true

    // ========================================================================
    // Construction / Destruction
    // ========================================================================
    Game();
    ~Game() = default;

    // ========================================================================
    // Core Game Loop
    // ========================================================================
    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // ========================================================================
    // Scene Management
    // ========================================================================
    void SetScene(GameScene scene);
    void UnloadScene();

    // ========================================================================
    // Actor Management
    // ========================================================================
    void InitializeActors();
    void UpdateActors(float deltaTime);
    void AddActor(Actor* actor);
    void RemoveActor(Actor* actor);

    // ========================================================================
    // Drawable Management
    // ========================================================================
    void AddDrawable(DrawComponent* drawable);
    void RemoveDrawable(DrawComponent* drawable);
    std::vector<DrawComponent*>& GetDrawables() { return mDrawables; }

    // ========================================================================
    // Getters
    // ========================================================================
    Renderer* GetRenderer() { return mRenderer; }
    Ship* GetShip() const { return mShip; }
    Ship* GetShip1() const { return mShip1; }
    Ship* GetShip2() const { return mShip2; }
    OpeningAudio* GetOpeningAudio() { return mOpeningAudio; }
    GameScene GetCurrentScene() const { return mCurrentScene; }

private:
    // ========================================================================
    // Main Loop Methods
    // ========================================================================
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    // ========================================================================
    // Input Handling
    // ========================================================================
    void ProcessOpeningInput(const SDL_Event& event);
    void ProcessMainMenuInput(const SDL_Event& event);
    void ProcessGameplayInput(const SDL_Event& event);
    void ProcessKeyboardState();

    // ========================================================================
    // Scene Update Methods
    // ========================================================================
    void UpdateOpeningScene(float deltaTime);
    void UpdateMainMenuScene(float deltaTime);
    void UpdateGameplayScene(float deltaTime);

    // ========================================================================
    // Video Management
    // ========================================================================
    void InitializeOpeningVideo();
    void UpdateVideo(float deltaTime);
    void HandleVideoTransition();
    void HandleAberturaAudioDelay();

    // ========================================================================
    // Rendering
    // ========================================================================
    void RenderOpeningScene();
    void RenderGameplayScene();

    // ========================================================================
    // Collision Detection
    // ========================================================================
    void CheckLaserCollisions();
    void CheckShipLaserCollision(Ship* ship, LaserBeam* laser, LaserBeamComponent* laserComp, Ship* ownerShip);

    // ========================================================================
    // Utility Methods
    // ========================================================================
    void RemoveActorFromVector(std::vector<Actor*>& actors, Actor* actor);

    // ========================================================================
    // Member Variables - Core Systems
    // ========================================================================
    SDL_Window* mWindow;
    Renderer* mRenderer;
    Uint32 mTicksCount;
    bool mIsRunning;
    bool mIsDebugging;
    bool mUpdatingActors;

    // ========================================================================
    // Member Variables - Actors and Drawables
    // ========================================================================
    std::vector<Actor*> mActors;
    std::vector<Actor*> mPendingActors;
    std::vector<DrawComponent*> mDrawables;

    // ========================================================================
    // Member Variables - Game Entities
    // ========================================================================
    Ship* mShip;
    Ship* mShip1;
    Ship* mShip2;

    // ========================================================================
    // Member Variables - Scene Management
    // ========================================================================
    GameScene mCurrentScene;

    // ========================================================================
    // Member Variables - Video System
    // ========================================================================
    VideoPlayer* mVideoPlayer;
    bool mShowingVideo;
    VideoState mVideoState;
    double mAberturaStartTime;
    double mAberturaAudioStartTime;
    bool mAberturaAudioPending;

    // ========================================================================
    // Member Variables - UI and Audio
    // ========================================================================
    MenuHUD* mMenuHUD;
    OpeningAudio* mOpeningAudio;
};
