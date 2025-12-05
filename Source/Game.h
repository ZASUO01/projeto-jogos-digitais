// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <chrono>
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

    // Game specific
    bool IsPlayerSet() const { return mIsPlayerSet; }
    void SetPlayer(const Vector2 &position, float rotation);
    Ship* GetPlayer() const { return mPlayer; }
    bool IsEnemySet(int id);
    void SetEnemy(int id, const Vector2 &position, float rotation);

    // Game State
    void SetPlayerState(const RawState& raw) const;
    void SetEnemiesState(const std::vector<OtherState> &others);
    void RemoveInactiveEnemies();

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
    bool mIsPlayerSet;
    std::unordered_map<int, Ship*> mEnemies;
    std::unordered_map<int, std::chrono::steady_clock::time_point> mEnemiesLastUpdate;
    static constexpr int ENEMY_RESPONSE_TIMEOUT_SECONDS = 2;
};
