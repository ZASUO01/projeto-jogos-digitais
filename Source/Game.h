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

enum class GameScene
{
    MainMenu,
    Level1
};

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

    // UI functions
    void PushUI(class UIScreen* screen) { mUIStack.emplace_back(screen); }
    const std::vector<class UIScreen*>& GetUIStack() { return mUIStack; }

    // Renderer
    class Renderer* GetRenderer() { return mRenderer; }

    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 768;

    // Draw functions
    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    std::vector<class DrawComponent*>& GetDrawables() { return mDrawables; }

    Ship *GetShip() const {return mShip; }

    // Scene Handling
    void SetScene(GameScene scene);
    void UnloadScene();

private:
    void ProcessInput();
    void UpdateGame(float deltaTime);
    void GenerateOutput();
    void RemoveActorFromVector(std::vector< Actor*> &actors,  Actor *actor);

    // All the actors in the game
    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;

    // All the draw components
    std::vector<class DrawComponent*> mDrawables;

    // All UI screens in the game
    std::vector<class UIScreen*> mUIStack;

    // SDL stuff
    SDL_Window* mWindow;
    class Renderer* mRenderer;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if we're updating actors right now
    bool mIsRunning;
    bool mIsDebugging;
    bool mUpdatingActors;

    // Game-specific
    Ship* mShip;
};
