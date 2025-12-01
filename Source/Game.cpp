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
#include "Video/VideoPlayer.h"
#include "UI/MenuHUD.h"
#include "Audio/OpeningAudio.h"
#include "Random.h"
#include <SDL.h>

Game::Game()
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mIsDebugging(false)
        ,mUpdatingActors(false)
        ,mVideoPlayer(nullptr)
        ,mShowingVideo(false)
        ,mVideoState(VideoState::Begin)
        ,mAberturaStartTime(0.0)
        ,mAberturaAudioStartTime(0.0)
        ,mAberturaAudioPending(false)
        ,mMenuHUD(nullptr)
        ,mOpeningAudio(nullptr)
        ,mShip(nullptr)
        ,mShip1(nullptr)
        ,mShip2(nullptr)
        ,mCurrentScene(GameScene::Opening)
{}

bool Game::Initialize()
{
    Random::Init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("TP2: Asteroids", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                               WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = new Renderer(mWindow);
    mRenderer->Initialize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));

    // Inicializar áudio da tela inicial (depois do contexto OpenGL)
    mOpeningAudio = new OpeningAudio();
    if (!mOpeningAudio->Initialize()) {
        SDL_Log("Failed to initialize opening audio");
    }

    mTicksCount = SDL_GetTicks();

    // Inicializar com a cena de abertura
    SetScene(GameScene::Opening);

    return true;
}

void Game::UnloadScene()
{
    // Limpar todos os atores
    for(auto *actor : mActors) {
        actor->SetState(ActorState::Destroy);
    }
    
    // Limpar atores pendentes
    for(auto *actor : mPendingActors) {
        actor->SetState(ActorState::Destroy);
    }
    
    // Processar destruição dos atores (fazer loop até que todos sejam destruídos)
    int maxIterations = 10; // Prevenir loop infinito
    while ((!mActors.empty() || !mPendingActors.empty()) && maxIterations > 0) {
        UpdateActors(0.0f);
        maxIterations--;
    }
    
    // Limpar drawables
    mDrawables.clear();
    
    // Parar e limpar vídeo
    if (mVideoPlayer) {
        mVideoPlayer->Stop();
        mVideoPlayer->Shutdown();
        delete mVideoPlayer;
        mVideoPlayer = nullptr;
    }
    mShowingVideo = false;
    
    // Parar áudio (mas não deletar, pois pode ser usado em outras cenas)
    if (mOpeningAudio) {
        mOpeningAudio->Stop();
    }
    
    // Limpar MenuHUD
    if (mMenuHUD) {
        delete mMenuHUD;
        mMenuHUD = nullptr;
    }
    
    // Resetar estados
    mShip = nullptr;
    mShip1 = nullptr;
    mShip2 = nullptr;
    mVideoState = VideoState::Begin;
    mAberturaStartTime = 0.0;
    mAberturaAudioStartTime = 0.0;
    mAberturaAudioPending = false;
}

void Game::SetScene(GameScene nextScene)
{
    UnloadScene();
    mCurrentScene = nextScene;
    
    switch (nextScene)
    {
        case GameScene::Opening:
            InitializeOpeningVideo();
            break;
        case GameScene::MainMenu:
            mMenuHUD = new MenuHUD();
            mMenuHUD->Show();
            break;
        case GameScene::Gameplay:
            InitializeActors();
            if (mOpeningAudio)
            {
                mOpeningAudio->Stop();
                // Tocar loop como som de fundo (volume 30 de 128 = ~23%)
                mOpeningAudio->PlayLoopBackground(true, 30);
            }
            break;
    }
}

void Game::InitializeOpeningVideo()
{
    if (DEBUG_MODE)
    {
        // Modo debug: pular direto para EntranceLoop
        mVideoPlayer = new VideoPlayer();
        if (mVideoPlayer->PlayVideo("Opening/entrance_loop.mp4", mWindow, true))
        {
            mShowingVideo = true;
            mVideoState = VideoState::EntranceLoop;
            if (mOpeningAudio)
            {
                mOpeningAudio->PlayLoop(true);
            }
            mMenuHUD = new MenuHUD();
            mMenuHUD->Show();
        }
        else
        {
            SDL_Log("Failed to load entrance_loop.mp4 in debug mode, starting game");
            delete mVideoPlayer;
            mVideoPlayer = nullptr;
            SetScene(GameScene::Gameplay);
        }
    }
    else
    {
        // Modo normal: começar com begin.mp4
        mVideoPlayer = new VideoPlayer();
        if (mVideoPlayer->PlayVideo("Opening/begin.mp4", mWindow, true))
        {
            mShowingVideo = true;
            mVideoState = VideoState::Begin;
            if (mOpeningAudio)
            {
                mOpeningAudio->PlayBegin(true);
            }
        }
        else
        {
            SDL_Log("Failed to load intro video, continuing without it");
            delete mVideoPlayer;
            mVideoPlayer = nullptr;
            SetScene(GameScene::Gameplay);
        }
    }
}

void Game::InitializeActors()
{
    new Floor(this);
    
    mShip1 = new Ship(this, 40, 300, 3, Vector3(0.0f, 0.7f, 0.7f), false);
    mShip1->SetPosition(Vector2(Game::WINDOW_WIDTH - 100, 100));
    
    mShip2 = new Ship(this, 40, 300, 3, Vector3(1.0f, 0.0f, 0.0f), true);
    mShip2->SetPosition(Vector2(100, Game::WINDOW_HEIGHT - 100));
    
    mShip = mShip1;
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
            case SDL_KEYDOWN:
                switch (mCurrentScene)
                {
                    case GameScene::Opening:
                        ProcessOpeningInput(event);
                        break;
                    case GameScene::MainMenu:
                        ProcessMainMenuInput(event);
                        break;
                    case GameScene::Gameplay:
                        ProcessGameplayInput(event);
                        break;
                }
                break;
            case SDL_TEXTINPUT:
                if (mMenuHUD && mMenuHUD->IsVisible())
                {
                    mMenuHUD->HandleTextInput(event.text.text);
                }
                break;
        }
    }

    // Processar estado do teclado apenas na cena de gameplay
    if (mCurrentScene == GameScene::Gameplay)
    {
        ProcessKeyboardState();
    }
}

void Game::ProcessOpeningInput(const SDL_Event& event)
{
    // Processar input do menu HUD se visível
    if (mMenuHUD && mMenuHUD->IsVisible())
    {
        mMenuHUD->HandleKeyPress(event.key.keysym.sym);
        
        if (mMenuHUD->WasStartSelected())
        {
            mMenuHUD->ResetStartSelected();
            SetScene(GameScene::Gameplay);
            return;
        }
    }
    
    // Processar transição de vídeo com Enter
    if (mShowingVideo && !(mMenuHUD && mMenuHUD->IsVisible()))
    {
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
        {
            HandleVideoTransition();
        }
    }
}

void Game::ProcessMainMenuInput(const SDL_Event& event)
{
    if (mMenuHUD && mMenuHUD->IsVisible())
    {
        mMenuHUD->HandleKeyPress(event.key.keysym.sym);
        
        if (mMenuHUD->WasStartSelected())
        {
            mMenuHUD->ResetStartSelected();
            SetScene(GameScene::Gameplay);
        }
    }
}

void Game::ProcessGameplayInput(const SDL_Event& event)
{
    // Input de gameplay é processado via ProcessKeyboardState
}

void Game::ProcessKeyboardState()
{
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    
    if (state[SDL_SCANCODE_ESCAPE])
    {
        Quit();
        return;
    }

    // Processar input de todos os atores
    for (auto* actor : mActors)
    {
        actor->ProcessInput(state);
    }
}

void Game::HandleVideoTransition()
{
    if (!mVideoPlayer)
    {
        return;
    }

    if (mVideoState == VideoState::Begin)
    {
        // Transição: Begin -> Abertura
        mVideoPlayer->Stop();
        if (mOpeningAudio)
        {
            mOpeningAudio->StopBegin();
        }
        
        if (mVideoPlayer->PlayVideo("Opening/abertura.mp4", mWindow, false))
        {
            mVideoState = VideoState::Abertura;
            mAberturaStartTime = SDL_GetTicks() / 1000.0;
            mAberturaAudioStartTime = mAberturaStartTime + 5.0;  // Delay de 5 segundos
            mAberturaAudioPending = true;
        }
        else
        {
            SDL_Log("Failed to load abertura.mp4, starting game");
            SetScene(GameScene::Gameplay);
        }
    }
    else if (mVideoState == VideoState::Abertura || mVideoState == VideoState::EntranceLoop)
    {
        // Pular vídeo e ir direto para o jogo
        SetScene(GameScene::Gameplay);
    }
}

void Game::UpdateGame()
{
    // Limitar frame rate a ~60 FPS
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    // Calcular delta time
    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;  // Limitar delta time máximo
    }

    mTicksCount = SDL_GetTicks();

    // Atualizar baseado na cena atual
    switch (mCurrentScene)
    {
        case GameScene::Opening:
            UpdateOpeningScene(deltaTime);
            break;
        case GameScene::MainMenu:
            UpdateMainMenuScene(deltaTime);
            break;
        case GameScene::Gameplay:
            UpdateGameplayScene(deltaTime);
            break;
    }
}

void Game::UpdateOpeningScene(float deltaTime)
{
    if (!mShowingVideo || !mVideoPlayer)
    {
        return;
    }

    // Atualizar vídeo
    mVideoPlayer->Update();
    
    // Verificar delay do áudio da abertura
    HandleAberturaAudioDelay();
    
    // Atualizar áudio (otimizado: verificar a cada 50ms)
    static Uint32 lastAudioCheck = 0;
    Uint32 currentTicks = SDL_GetTicks();
    if (currentTicks - lastAudioCheck > 50)
    {
        if (mOpeningAudio)
        {
            mOpeningAudio->Update();
        }
        lastAudioCheck = currentTicks;
    }
    
    // Mostrar menu após 10 segundos do vídeo de abertura
    if (mVideoState == VideoState::Abertura && mMenuHUD)
    {
        double elapsedTime = (SDL_GetTicks() / 1000.0) - mAberturaStartTime;
        if (elapsedTime >= 10.0 && !mMenuHUD->IsVisible())
        {
            mMenuHUD->Show();
        }
    }
    
    // Atualizar menu HUD se visível
    if (mMenuHUD && mMenuHUD->IsVisible())
    {
        mMenuHUD->Update(deltaTime);
    }
    
    // Transição automática: Abertura -> EntranceLoop
    if (mVideoState == VideoState::Abertura && mVideoPlayer->HasFinished())
    {
        if (mVideoPlayer->PlayVideo("Opening/entrance_loop.mp4", mWindow, true))
        {
            mVideoState = VideoState::EntranceLoop;
            if (mOpeningAudio)
            {
                mOpeningAudio->StopAbertura();
                mOpeningAudio->PlayLoop(true);
            }
        }
        else
        {
            SDL_Log("Failed to load entrance_loop.mp4, starting game");
            SetScene(GameScene::Gameplay);
        }
    }
}

void Game::UpdateMainMenuScene(float deltaTime)
{
    if (mMenuHUD && mMenuHUD->IsVisible())
    {
        mMenuHUD->Update(deltaTime);
    }
}

void Game::UpdateGameplayScene(float deltaTime)
{
    // Atualizar áudio
    if (mOpeningAudio)
    {
        mOpeningAudio->Update();
    }
    
    // Atualizar todos os atores
    UpdateActors(deltaTime);
}

void Game::HandleAberturaAudioDelay()
{
    if (mVideoState == VideoState::Abertura && mAberturaAudioPending)
    {
        double currentTime = SDL_GetTicks() / 1000.0;
        if (currentTime >= mAberturaAudioStartTime)
        {
            if (mOpeningAudio)
            {
                mOpeningAudio->PlayAbertura(false);
            }
            mAberturaAudioPending = false;
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

    CheckLaserCollisions();

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
    switch (mCurrentScene)
    {
        case GameScene::Opening:
            RenderOpeningScene();
            break;
        case GameScene::MainMenu:
            // Menu é renderizado via SDL no MenuHUD
            break;
        case GameScene::Gameplay:
            RenderGameplayScene();
            break;
    }
}

void Game::RenderOpeningScene()
{
    if (!mShowingVideo || !mVideoPlayer)
    {
        return;
    }

    // Renderizar vídeo
    mVideoPlayer->RenderWithoutPresent();
    
    // Renderizar menu HUD sobre o vídeo se visível
    if (mMenuHUD && mMenuHUD->IsVisible())
    {
        SDL_Renderer* videoRenderer = mVideoPlayer->GetRenderer();
        if (videoRenderer)
        {
            mMenuHUD->Render(videoRenderer);
        }
    }
    
    // Apresentar frame
    SDL_Renderer* videoRenderer = mVideoPlayer->GetRenderer();
    if (videoRenderer)
    {
        SDL_RenderPresent(videoRenderer);
    }
}

void Game::RenderGameplayScene()
{
    // Limpar buffer
    mRenderer->Clear();
    
    // Desenhar grid de fundo
    float currentTime = SDL_GetTicks() / 1000.0f;
    mRenderer->DrawAdvancedGrid(
        mRenderer->GetScreenWidth(), 
        mRenderer->GetScreenHeight(), 
        currentTime
    );

    // Desenhar todos os drawables
    for (auto* drawable : mDrawables)
    {
        drawable->Draw(mRenderer);
        
        // Desenhar debug se habilitado
        if (mIsDebugging)
        {
            for (auto* component : drawable->GetOwner()->GetComponents())
            {
                component->DebugDraw(mRenderer);
            }
        }
    }

    // Apresentar frame
    mRenderer->Present();
}

void Game::Shutdown()
{
    // Descarregar cena atual antes de limpar tudo
    UnloadScene();
    
    // Limpar recursos restantes
    if (mOpeningAudio) {
        mOpeningAudio->Shutdown();
        delete mOpeningAudio;
        mOpeningAudio = nullptr;
    }

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

void Game::CheckLaserCollisions()
{
    for (auto* actor : mActors)
    {
        LaserBeam* laser = dynamic_cast<LaserBeam*>(actor);
        if (!laser || laser->GetState() != ActorState::Active)
        {
            continue;
        }

        LaserBeamComponent* laserComp = laser->GetLaserComponent();
        Ship* ownerShip = laser->GetOwnerShip();
        
        if (!laserComp || !laserComp->IsActive())
        {
            continue;
        }

        // Verificar colisão com Ship1
        CheckShipLaserCollision(mShip1, laser, laserComp, ownerShip);
        
        // Verificar colisão com Ship2
        CheckShipLaserCollision(mShip2, laser, laserComp, ownerShip);
    }
}

void Game::CheckShipLaserCollision(Ship* ship, LaserBeam* laser, LaserBeamComponent* laserComp, Ship* ownerShip)
{
    if (!ship || 
        ship->GetState() != ActorState::Active || 
        ship == ownerShip || 
        ship->IsInvincible())
    {
        return;
    }

    CircleColliderComponent* collider = ship->GetComponent<CircleColliderComponent>();
    if (!collider)
    {
        return;
    }

    if (laserComp->IntersectCircle(ship->GetPosition(), collider->GetRadius()))
    {
        ship->TakeDamage();
    }
}