//
// Created by pedro-souza on 23/11/2025.
//

// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include <algorithm>
#include <vector>
#include "Game.h"
#include "Actors/Ship.h"
#include "Components/DrawComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Random.h"

Game::Game()
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mIsDebugging(false)
        ,mUpdatingActors(false)
        ,mShip(nullptr)
        ,mClient(nullptr)
        ,mNetTicksCount(0)
{}

bool Game::Initialize()
{
    Random::Init();

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("TP2: Asteroids", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = new Renderer(mWindow);
    mRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    mClient = new Client(this);
    mClient->Initialize();
    mClient->AddServerAddr("192.168.1.13");
    mClient->Connect();

    // Init all game actors
    InitializeActors();

    mTicksCount = SDL_GetTicks();
    mNetTicksCount = SDL_GetTicks();

    return true;
}

void Game::InitializeActors()
{
    mShip = new Ship(this, 40, 300, 3);
    mShip->SetPosition(Vector2(Game::WINDOW_WIDTH / 2, Game::WINDOW_HEIGHT / 2));
}

void Game::RunLoop()
{
    while (mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);

    unsigned int size = mActors.size();
    for (unsigned int i = 0; i < size; ++i) {
        mActors[i]->ProcessInput(state);
    }
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    if (SDL_TICKS_PASSED(SDL_GetTicks(), mNetTicksCount + 66)) {
        mClient->SendCommandsToServer();
        mNetTicksCount = SDL_GetTicks();
    }
    mClient->ReceiveDataFromServer();


    // Update all actors and pending actors
    //UpdateActors(deltaTime);
}

void Game::UpdateActors(float deltaTime)
{
    mUpdatingActors = true;
    for (auto &actor : mActors) {
        actor->Update(deltaTime);
    }
    mUpdatingActors = false;

    for (auto &pending: mPendingActors) {
        mActors.emplace_back(pending);
    }
    mPendingActors.clear();

    std::vector<Actor*> deadActors;
    for (auto &actor : mActors) {
        if (actor->GetState() == ActorState::Destroy) {
            deadActors.emplace_back(actor);
        }
    }

    for (auto actor : deadActors) {
        delete actor;
    }

}

void Game::AddActor(Actor* actor)
{
    if (mUpdatingActors) {
        mPendingActors.emplace_back(actor);
    }else {
        mActors.emplace_back(actor);
    }
}

void Game::RemoveActor(Actor* actor)
{
    RemoveActorFromVector(mActors, actor);
    RemoveActorFromVector(mPendingActors, actor);
}

void Game::AddDrawable(class DrawComponent *drawable)
{
    mDrawables.emplace_back(drawable);
    std::sort(
        mDrawables.begin(),
        mDrawables.end(),
        [](const DrawComponent *a, const DrawComponent *b) {
            return a->GetDrawOrder() < b->GetDrawOrder();
        });
}

void Game::RemoveDrawable(class DrawComponent *drawable)
{
    std::vector<class DrawComponent *>::iterator it;
    it = std::find(mDrawables.begin(), mDrawables.end(), drawable);
    if (it != mDrawables.end()) {
        mDrawables.erase(it);
    }
}

void Game::GenerateOutput()
{
    // Clear back buffer
    mRenderer->Clear();

    unsigned int size = mDrawables.size();
    unsigned int size2;
    for (unsigned int i = 0; i < size; i++) {
        mDrawables[i]->Draw(mRenderer);
        if (mIsDebugging) {
            size2 = mDrawables[i]->GetOwner()->GetComponents().size();
            for (unsigned int j = 0; j < size2; j++) {
                mDrawables[i]->GetOwner()->GetComponents()[j]->DebugDraw(mRenderer);
            }
        }

    }

    // Swap front buffer and back buffer
    mRenderer->Present();
}

void Game::Shutdown()
{
    std::vector<Actor*> actorsToDelete = mActors;
    mActors.clear();

    for (auto actor : actorsToDelete) {
        delete actor;
    }

    std::vector<Actor*> pendingToDelete = mPendingActors;
    mPendingActors.clear();
    for (auto actor : pendingToDelete) {
        delete actor;
    }

    mDrawables.clear();

    mRenderer->Shutdown();
    delete mRenderer;
    mRenderer = nullptr;

    mClient->Disconnect();
    mClient->Shutdown();

    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void Game::RemoveActorFromVector(std::vector<class Actor*> &actors, class Actor *actor) {
    auto it = std::find(actors.begin(), actors.end(), actor);
    if (it != actors.end()) {
        std::iter_swap(it, actors.end() - 1);
        actors.pop_back();
    }
}