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
#include "Actors/Floor.h"
#include "Actors/LaserBeam.h"
#include "Components/DrawComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/LaserBeamComponent.h"
#include "Components/CircleColliderComponent.h"
#include "Random.h"

Game::Game()
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mIsDebugging(false)
        ,mUpdatingActors(false)
        ,mClient(nullptr)
        ,mNetTicksCount(0)
        ,mPlayer(nullptr)
        ,mIsPlayerSet(false)
{}

bool Game::Initialize(){
    Random::Init();

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("Line Casters", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!mWindow){
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = new Renderer(mWindow);
    if (!mRenderer->Initialize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT))) {
        SDL_Log("Failed to initialize renderer.");
        return false;
    }

    mClient = new Client(this);
    mClient->Initialize();
    mClient->AddServerAddr("192.168.1.13");
    mClient->Connect();

    // Init all game actors
    InitFloor();

    mTicksCount = SDL_GetTicks();
    mIsRunning = mTicksCount;

    return true;
}

void Game::InitFloor(){
    new Floor(this);
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

void Game::ProcessInput(){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        switch (event.type){
            case SDL_QUIT:
                Quit();
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    Quit();
                }
                break;
            default:
                break;
        }
    }
    const Uint8* state = SDL_GetKeyboardState(nullptr);

    // Store commands
    mClient->AddInput(state);

    // Execute prediction
    if (mIsPlayerSet) {
        mPlayer->ProcessInput(state);
    }
}

void Game::UpdateGame(){
    // Execute prediction
    if (mIsPlayerSet) {
        mPlayer->Update(SIM_DELTA_TIME);
    }

    // Receive packets
    mClient->ReceiveStateFromServer();

    // execute enemies state interpolation
    InterpolateEnemies();

    // control enemies list
    RemoveInactiveEnemies();

    // Wait 100ms to send the next inputs batch
    if (SDL_TICKS_PASSED(SDL_GetTicks(), mNetTicksCount + 100)) {
        mClient->SendCommandsToServer();
        mNetTicksCount = SDL_GetTicks();
    }

    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}
    mTicksCount = SDL_GetTicks();
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
    mRenderer->Clear();

    float currentTime = SDL_GetTicks() / 1000.0f;
    mRenderer->DrawAdvancedGrid(mRenderer->GetScreenWidth(),
                                mRenderer->GetScreenHeight(),
                                currentTime);

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

void Game::CheckLaserCollisions()
{
    /*
    for (auto actor : mActors) {
        LaserBeam* laser = dynamic_cast<LaserBeam*>(actor);
        if (laser && laser->GetState() == ActorState::Active) {
            LaserBeamComponent* laserComp = laser->GetLaserComponent();
            Ship* ownerShip = laser->GetOwnerShip();
            if (laserComp && laserComp->IsActive()) {
                if (mShip1 && mShip1->GetState() == ActorState::Active && mShip1 != ownerShip && !mShip1->IsInvincible()) {
                    CircleColliderComponent* collider = mShip1->GetComponent<CircleColliderComponent>();
                    if (collider) {
                        if (laserComp->IntersectCircle(mShip1->GetPosition(), collider->GetRadius())) {
                            mShip1->TakeDamage();
                        }
                    }
                }

                if (mShip2 && mShip2->GetState() == ActorState::Active && mShip2 != ownerShip && !mShip2->IsInvincible()) {
                    CircleColliderComponent* collider = mShip2->GetComponent<CircleColliderComponent>();
                    if (collider) {
                        if (laserComp->IntersectCircle(mShip2->GetPosition(), collider->GetRadius())) {
                            mShip2->TakeDamage();
                        }
                    }
                }
            }
        }
    }
    */
}

void Game::SetPlayer(const Vector2 &position, const float rotation) {
    if (mIsPlayerSet) {
        return;
    }

    mPlayer = new Ship(this, 50);
    mPlayer->SetPosition(position);
    mPlayer->SetRotation(rotation);
    mIsPlayerSet = true;
}

bool Game::IsEnemySet(const int id) {
    if (mEnemies.find(id) == mEnemies.end()) {
        return false;
    }
    return true;
}

void Game::SetEnemy(const int id,const Vector2 &position, const float rotation) {
        auto enemy = new Ship(this, 50);
        enemy->SetPosition(position);
        enemy->SetRotation(rotation);
        mEnemies.emplace(id, enemy);
        mEnemiesLastUpdate.emplace(id, std::chrono::steady_clock::now());
}

void Game::SetPlayerState(const RawState& raw) const {
    const auto newPlayerPos = Vector2(raw.posX, raw.posY);
    const auto rotation = raw.rotation;

    mPlayer->SetPosition(newPlayerPos);
    mPlayer->SetRotation(rotation);
}

void Game::SetEnemiesState(const std::vector<OtherState> &others)  {
    const auto now = std::chrono::steady_clock::now();

    for (const auto& other : others) {
        if (const auto it = mEnemies.find(other.id); it != mEnemies.end()) {
            const auto enemy = it->second;

            mEnemyStateBuffers[other.id].emplace_back(
                other.posX,
                other.posY,
                other.rotation,
                now
            );

            auto& buffer = mEnemyStateBuffers[other.id];
            if (buffer.size() > 5) {
                buffer.erase(buffer.begin(), buffer.begin() + buffer.size() - 5);
            }

            if (auto it1 = mEnemiesLastUpdate.find(other.id); it1 != mEnemiesLastUpdate.end()) {
                it1->second = std::chrono::steady_clock::now();
            }
        }
    }
}

void Game::InterpolateEnemies() {
    const auto now = std::chrono::steady_clock::now();
    const auto interpolationDelay = std::chrono::milliseconds(static_cast<int>(INTERPOLATION_DELAY_MS));
    const auto targetTime = now - interpolationDelay;

    for (auto const& [id, enemy] : mEnemies) {
        auto& buffer = mEnemyStateBuffers[id];

        if (buffer.size() < 2) {
            continue;
        }

        auto itA = std::find_if(
            buffer.rbegin(),
            buffer.rend(),
            [&targetTime](const InterpolatedState& state) {
                return state.receptionTime <= targetTime;
            }
        );

        if (itA == buffer.rend()) {
            auto pos = Vector2(buffer.front().posX, buffer.front().posY);

            enemy->SetPosition(pos);
            enemy->SetRotation(buffer.front().rotation);
            continue;
        }


        auto itB = itA.base();
        if (itB == buffer.end()) {
            auto pos = Vector2(buffer.back().posX, buffer.back().posY);

            enemy->SetPosition(pos);
            enemy->SetRotation(buffer.back().rotation);
            continue;
        }

        // calculate the interpolation factor
        const InterpolatedState& stateA = *itA;
        const InterpolatedState& stateB = *itB;


        const auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(stateB.receptionTime - stateA.receptionTime).count();
        const auto elapsedDuration = std::chrono::duration_cast<std::chrono::microseconds>(targetTime - stateA.receptionTime).count();

        float k = 0.0f;
        if (totalDuration > 0) {
            k = static_cast<float>(elapsedDuration) / totalDuration;
        }


        const float interpolatedPosX =
            stateA.posX + (stateB.posX - stateA.posX) * k;

        const float interpolatedPosY =
            stateA.posY + (stateB.posY - stateA.posY) * k;

        const float interpolatedRotation =
            stateA.rotation + (stateB.rotation - stateA.rotation) * k;

        enemy->SetPosition(Vector2{interpolatedPosX, interpolatedPosY});
        enemy->SetRotation(interpolatedRotation);

        if (buffer.front().receptionTime < targetTime - std::chrono::milliseconds(200)) {
            buffer.erase(buffer.begin());
        }
    }
}

void Game::RemoveInactiveEnemies() {
    const auto now = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::milliseconds(ENEMY_RESPONSE_TIMEOUT_MS);

    for (auto it = mEnemiesLastUpdate.begin(); it != mEnemiesLastUpdate.end();) {
        if (const auto secondsPassed= std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second); secondsPassed > timeout) {
            if (const auto enemyIt = mEnemies.find(it->first); enemyIt!= mEnemies.end()) {
                delete enemyIt->second;
                mEnemies.erase(enemyIt);
            }

            it = mEnemiesLastUpdate.erase(it);
        } else {
            ++it;
        }
    }
}
