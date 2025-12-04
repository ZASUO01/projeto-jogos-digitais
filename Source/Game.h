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
#include <unordered_map>
#include "Actors/Ship.h"
#include "Actors/Actor.h"
#include "Renderer/Renderer.h"
#include "../Client/Client.h"

class Game{
public:
    Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    void InitFloor();
    void ActorsInput(const Uint8 *state) const;
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);

    class Renderer* GetRenderer() { return mRenderer; }

    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 768;
    static const int RENDER_WIDTH = 1024;
    static const int RENDER_HEIGHT = 768;
    static constexpr float SIM_DELTA_TIME = 1.0f / 60.0f;

    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    std::vector<class DrawComponent*>& GetDrawables() { return mDrawables; }

    // Network specific
    void SetAuthoritativeState(const RawState& raw, const std::vector<OtherState> &others) const;

    // Game specific
    void SetPlayer(const Vector2 &position);
    Ship* GetPlayer() const { return mPlayer; }
    void SetEnemy(int id, const Vector2 &position);
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


    // Network specific
    class Client *mClient;
    Uint32 mNetTicksCount;

    // Game specific
    Ship* mPlayer;
    std::unordered_map<int, Ship*> mEnemies;
};
