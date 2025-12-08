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
#include "UI/Screens/MainMenu.h"
#include "UI/Screens/GameOver.h"
#include "UI/Screens/UIScreen.h"
#include "UI/Screens/OpeningScreen.h"

Game::Game()
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mIsDebugging(false)
        ,mUpdatingActors(false)
        ,mShip(nullptr)
{}

bool Game::Initialize()
{
    Random::Init();

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    // Criar janela com flags para permitir maximizar
    mWindow = SDL_CreateWindow("TP2: Asteroids", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                WINDOW_WIDTH, WINDOW_HEIGHT, 
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    // Maximizar a janela
    SDL_MaximizeWindow(mWindow);

    // Obter o tamanho real da janela após maximizar
    int actualWidth, actualHeight;
    SDL_GetWindowSize(mWindow, &actualWidth, &actualHeight);

    mRenderer = new Renderer(mWindow);
    mRenderer->Initialize(actualWidth, actualHeight);

    // Iniciar com a tela de abertura (vídeo)
    new OpeningScreen(this);

    mTicksCount = SDL_GetTicks();

    return true;
}

void Game::InitializeActors()
{
    mShip = new Ship(this, 40, 300, 3);
    // Posicionar a nave no centro da janela atual
    int windowWidth = GetWindowWidth();
    int windowHeight = GetWindowHeight();
    mShip->SetPosition(Vector2(static_cast<float>(windowWidth) / 2.0f, static_cast<float>(windowHeight) / 2.0f));
}

int Game::GetWindowWidth() const
{
    int width, height;
    SDL_GetWindowSize(mWindow, &width, &height);
    return width;
}

int Game::GetWindowHeight() const
{
    int width, height;
    SDL_GetWindowSize(mWindow, &width, &height);
    return height;
}

void Game::RunLoop()
{
    while (mIsRunning)
    {
        // Calculate delta time in seconds
        float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
        if (deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        mTicksCount = SDL_GetTicks();

        ProcessInput();
        UpdateGame(deltaTime);
        GenerateOutput();

        // Sleep to maintain frame rate
        int sleepTime = (1000 / 60) - (SDL_GetTicks() - mTicksCount);
        if (sleepTime > 0)
        {
            SDL_Delay(sleepTime);
        }
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
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    // Atualizar o renderer quando a janela for redimensionada
                    int newWidth = event.window.data1;
                    int newHeight = event.window.data2;
                    mRenderer->UpdateScreenSize(static_cast<float>(newWidth), static_cast<float>(newHeight));
                }
                break;
            case SDL_KEYDOWN:
                // Check if 'P' key is pressed to show Game Over screen (only in Level1)
                if (event.key.keysym.sym == SDLK_p && mUIStack.empty()) {
                    // Check if we're in Level1 by checking if ship exists
                    if (mShip) {
                        new GameOver(this, "../Assets/Fonts/Arial.ttf");
                    }
                }
                // Handle key press for UI screens
                if (!mUIStack.empty()) {
                    mUIStack.back()->HandleKeyPress(event.key.keysym.sym);
                }
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);

    unsigned int size = mActors.size();
    for (unsigned int i = 0; i < size; ++i) {
        mActors[i]->ProcessInput(state);
    }
}

void Game::UpdateGame(float deltaTime)
{
    // Calculate delta time in seconds
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    // Update all actors and pending actors
    UpdateActors(deltaTime);

    // Update UI screens
    for (auto ui : mUIStack) {
        if (ui->GetState() == UIScreen::UIState::Active) {
            ui->Update(deltaTime);
        }
    }

    // Delete any UI that are closed
    auto iter = mUIStack.begin();
    while (iter != mUIStack.end()) {
        if ((*iter)->GetState() == UIScreen::UIState::Closing) {
            delete *iter;
            iter = mUIStack.erase(iter);
        } else {
            ++iter;
        }
    }
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

    // Draw UI screens
    // Se houver OpeningScreen, desenhar o vídeo primeiro
    for (auto ui : mUIStack) {
        if (ui->GetState() == UIScreen::UIState::Active) {
            OpeningScreen* openingScreen = dynamic_cast<OpeningScreen*>(ui);
            if (openingScreen) {
                openingScreen->Draw(mRenderer);
                // Não desenhar outros elementos UI quando estiver na tela de abertura
                mRenderer->Present();
                return;
            }
            
        }
    }
    
    mRenderer->Draw();

    // Swap front buffer and back buffer
    mRenderer->Present();
}

void Game::UnloadScene()
{
    // Use state so we can call this from within an actor update
    for(auto *actor : mActors) {
        actor->SetState(ActorState::Destroy);
    }

    // Delete UI screens
    for (auto ui : mUIStack) {
        delete ui;
    }
    mUIStack.clear();
}

void Game::SetScene(GameScene nextScene)
{
    UnloadScene();

    switch (nextScene)
    {
        case GameScene::MainMenu:
        {
            new MainMenu(this, "../Assets/Fonts/Arial.ttf");
            break;
        }
        case GameScene::Level1:
        {
            // Init all game actors
            InitializeActors();
            break;
        }
    }
}

void Game::Shutdown()
{
    // Because ~Actor calls RemoveActor, have to use a different style loop
    while (!mActors.empty()) {
        delete mActors.back();
    }

    // Delete UI screens
    for (auto ui : mUIStack) {
        delete ui;
    }
    mUIStack.clear();

    mDrawables.clear();

    mRenderer->Shutdown();
    delete mRenderer;
    mRenderer = nullptr;

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