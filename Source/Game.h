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
    
    // Window
    SDL_Window* GetWindow() { return mWindow; }
    int GetWindowWidth() const;
    int GetWindowHeight() const;

    // Aumente a resolução base
    static const int WINDOW_WIDTH = 1920;  // Aumentado de 1280
    static const int WINDOW_HEIGHT = 1080; // Aumentado de 720

    // Adicione suporte a diferentes resoluções
    static const int RENDER_WIDTH = 1920;
    static const int RENDER_HEIGHT = 1080;

    // Draw functions
    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    std::vector<class DrawComponent*>& GetDrawables() { return mDrawables; }

    Ship *GetShip() const {return mShip; }
    Ship *GetShip1() const {return mShip1; }
    Ship *GetShip2() const {return mShip2; }

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
    Ship* mShip;  // Mantido para compatibilidade
    Ship* mShip1; // Nave 1 (canto superior direito)
    Ship* mShip2; // Nave 2 (canto inferior esquerdo)
};
