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
#include "Video/VideoPlayer.h"
#include "UI/MenuHUD.h"
#include "Audio/OpeningAudio.h"
#include "Random.h"

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
{}

bool Game::Initialize()
{
    Random::Init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("TP2: Asteroids", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = new Renderer(mWindow);
    mRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Inicializar áudio da tela inicial (depois do contexto OpenGL)
    mOpeningAudio = new OpeningAudio();
    if (!mOpeningAudio->Initialize()) {
        SDL_Log("Failed to initialize opening audio");
    }

    // Tentar carregar e tocar vídeo introdutório
    mVideoPlayer = new VideoPlayer();
    if (mVideoPlayer->PlayVideo("Opening/begin.mp4", mWindow, true)) {
        mShowingVideo = true;
        // Tocar áudio begin.mp3 junto com o vídeo
        mOpeningAudio->PlayBegin(true);
    } else {
        SDL_Log("Failed to load intro video, continuing without it");
        delete mVideoPlayer;
        mVideoPlayer = nullptr;
        InitializeActors();
    }

    mMenuHUD = new MenuHUD();

    mTicksCount = SDL_GetTicks();

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
            case SDL_KEYDOWN:
                if (mMenuHUD && mMenuHUD->IsVisible()) {
                    mMenuHUD->HandleKeyPress(event.key.keysym.sym);
                    
                    if (mMenuHUD->WasStartSelected()) {
                        mMenuHUD->ResetStartSelected();
                        if (mVideoPlayer) {
                            mVideoPlayer->Stop();
                        }
                        if (mOpeningAudio) {
                            mOpeningAudio->Stop();
                        }
                        mShowingVideo = false;
                        if (mActors.empty()) {
                            InitializeActors();
                        }
                        break;
                    }
                }
                
                if (mShowingVideo && !(mMenuHUD && mMenuHUD->IsVisible()) && 
                    (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)) {
                    if (mVideoPlayer) {
                        if (mVideoState == VideoState::Begin) {
                            // Parar begin.mp4 e tocar abertura.mp4
                            mVideoPlayer->Stop();
                            if (mOpeningAudio) {
                                mOpeningAudio->StopBegin();
                            }
                            if (mVideoPlayer->PlayVideo("Opening/abertura.mp4", mWindow, false)) {
                                mVideoState = VideoState::Abertura;
                                mAberturaStartTime = SDL_GetTicks() / 1000.0;
                                // Atrasar o início do áudio em 0.2 segundos
                                mAberturaAudioStartTime = mAberturaStartTime + 5;
                                mAberturaAudioPending = true;
                            } else {
                                SDL_Log("Failed to load abertura.mp4, starting game");
                                mShowingVideo = false;
                                if (mOpeningAudio) {
                                    mOpeningAudio->Stop();
                                }
                                if (mActors.empty()) {
                                    InitializeActors();
                                }
                            }
                        } else if (mVideoState == VideoState::Abertura || mVideoState == VideoState::EntranceLoop) {
                            // Parar qualquer vídeo e iniciar jogo
                            mVideoPlayer->Stop();
                            if (mOpeningAudio) {
                                mOpeningAudio->Stop();
                            }
                            mShowingVideo = false;
                            if (mActors.empty()) {
                                InitializeActors();
                            }
                        }
                    }
                }
                break;
            case SDL_TEXTINPUT:
                if (mMenuHUD && mMenuHUD->IsVisible()) {
                    mMenuHUD->HandleTextInput(event.text.text);
                }
                break;
        }
    }

    // Se estiver mostrando vídeo, não processar input dos atores
    if (mShowingVideo) {
        return;
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

    // Se estiver mostrando vídeo, atualizar apenas o vídeo
    if (mShowingVideo && mVideoPlayer) {
        mVideoPlayer->Update();
        
        // Verificar se precisa iniciar o áudio da abertura com delay
        if (mVideoState == VideoState::Abertura && mAberturaAudioPending) {
            double currentTime = SDL_GetTicks() / 1000.0;
            if (currentTime >= mAberturaAudioStartTime) {
                // Tocar áudio abertura.wav com delay
                if (mOpeningAudio) {
                    mOpeningAudio->PlayAbertura(false);
                }
                mAberturaAudioPending = false;
            }
        }
        
        // Atualizar áudio (verificar loops) - apenas quando necessário
        // Otimização: verificar áudio menos frequentemente para não impactar performance do vídeo
        static Uint32 lastAudioCheck = 0;
        Uint32 currentTicks = SDL_GetTicks();
        if (currentTicks - lastAudioCheck > 50) { // Verificar a cada 50ms em vez de a cada frame (~16ms)
            if (mOpeningAudio) {
                mOpeningAudio->Update();
            }
            lastAudioCheck = currentTicks;
        }
        
        if (mVideoState == VideoState::Abertura && mMenuHUD) {
            double currentTime = (SDL_GetTicks() / 1000.0) - mAberturaStartTime;
            if (currentTime >= 10.0 && !mMenuHUD->IsVisible()) {
                mMenuHUD->Show();
            }
        }
        
        if (mMenuHUD && mMenuHUD->IsVisible()) {
            mMenuHUD->Update(deltaTime);
        }
        
        if (mVideoState == VideoState::Abertura && mVideoPlayer->HasFinished()) {
            if (mVideoPlayer->PlayVideo("Opening/entrance_loop.mp4", mWindow, true)) {
                mVideoState = VideoState::EntranceLoop;
                // Tocar áudio loop.mp3 junto com o vídeo
                if (mOpeningAudio) {
                    mOpeningAudio->StopAbertura();
                    mOpeningAudio->PlayLoop(true);
                }
            } else {
                SDL_Log("Failed to load entrance_loop.mp4, starting game");
                mShowingVideo = false;
                if (mOpeningAudio) {
                    mOpeningAudio->Stop();
                }
                if (mActors.empty()) {
                    InitializeActors();
                }
            }
        }
        
        return;
    }

    // Update all actors and pending actors
    UpdateActors(deltaTime);
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
    if (mShowingVideo && mVideoPlayer) {
        mVideoPlayer->RenderWithoutPresent();
        
        if (mMenuHUD && mMenuHUD->IsVisible()) {
            SDL_Renderer* videoRenderer = mVideoPlayer->GetRenderer();
            if (videoRenderer) {
                mMenuHUD->Render(videoRenderer);
            }
        }
        
        SDL_Renderer* videoRenderer = mVideoPlayer->GetRenderer();
        if (videoRenderer) {
            SDL_RenderPresent(videoRenderer);
        }
        return;
    }

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

    if (mMenuHUD) {
        delete mMenuHUD;
        mMenuHUD = nullptr;
    }

    if (mOpeningAudio) {
        mOpeningAudio->Shutdown();
        delete mOpeningAudio;
        mOpeningAudio = nullptr;
    }

    if (mVideoPlayer) {
        mVideoPlayer->Shutdown();
        delete mVideoPlayer;
        mVideoPlayer = nullptr;
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