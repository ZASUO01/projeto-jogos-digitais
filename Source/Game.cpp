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
        ,mShip1(nullptr)
        ,mShip2(nullptr)
{}

// Inicializa o jogo, criando a janela SDL, o renderer e a tela de abertura
bool Game::Initialize()
{
    Random::Init();

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("TP2: Asteroids", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                WINDOW_WIDTH, WINDOW_HEIGHT, 
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    SDL_MaximizeWindow(mWindow);

    int actualWidth, actualHeight;
    SDL_GetWindowSize(mWindow, &actualWidth, &actualHeight);

    mRenderer = new Renderer(mWindow);
    if (!mRenderer->Initialize(actualWidth, actualHeight)) {
        SDL_Log("Failed to initialize renderer.");
        return false;
    }

    new OpeningScreen(this);

    mTicksCount = SDL_GetTicks();

    return true;
}

// Inicializa os atores do jogo: chão e duas naves
void Game::InitializeActors()
{
    new Floor(this);
    
    int windowWidth = GetWindowWidth();
    int windowHeight = GetWindowHeight();
    
    mShip1 = new Ship(this, 40, 300, 3, Vector3(0.0f, 0.7f, 0.7f), false);
    mShip1->SetPosition(Vector2(windowWidth - 100, 100));
    
    mShip2 = new Ship(this, 40, 300, 3, Vector3(1.0f, 0.0f, 0.0f), true);
    mShip2->SetPosition(Vector2(100, windowHeight - 100));
    
    mShip = mShip1;
}

// Retorna a largura atual da janela
int Game::GetWindowWidth() const
{
    int width, height;
    SDL_GetWindowSize(mWindow, &width, &height);
    return width;
}

// Retorna a altura atual da janela
int Game::GetWindowHeight() const
{
    int width, height;
    SDL_GetWindowSize(mWindow, &width, &height);
    return height;
}

// Executa o loop principal do jogo até que mIsRunning seja false
void Game::RunLoop()
{
    while (mIsRunning)
    {
        float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
        if (deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        mTicksCount = SDL_GetTicks();

        ProcessInput();
        UpdateGame(deltaTime);
        GenerateOutput();

        int sleepTime = (1000 / 60) - (SDL_GetTicks() - mTicksCount);
        if (sleepTime > 0)
        {
            SDL_Delay(sleepTime);
        }
    }
}

// Processa eventos de entrada do usuário (teclado, mouse, janela)
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
                    int newWidth = event.window.data1;
                    int newHeight = event.window.data2;
                    mRenderer->UpdateScreenSize(static_cast<float>(newWidth), static_cast<float>(newHeight));
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_p && mUIStack.empty()) {
                    if (mShip) {
                        new GameOver(this, "../Assets/Fonts/Arial.ttf", true);
                    }
                }
                if (!mUIStack.empty()) {
                    mUIStack.back()->HandleKeyPress(event.key.keysym.sym);
                }
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);
    
    if (state[SDL_SCANCODE_ESCAPE])
    {
        Quit();
    }

    unsigned int size = mActors.size();
    for (unsigned int i = 0; i < size; ++i) {
        mActors[i]->ProcessInput(state);
    }
}

// Atualiza a lógica do jogo: atores, UI e remove elementos fechados
void Game::UpdateGame(float deltaTime)
{
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    UpdateActors(deltaTime);

    for (auto ui : mUIStack) {
        if (ui->GetState() == UIScreen::UIState::Active) {
            ui->Update(deltaTime);
        }
    }

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

// Atualiza todos os atores, verifica colisões de laser e condições de vitória
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

    if (mUIStack.empty()) {
        if (mShip1 && mShip1->GetState() == ActorState::Active && mShip1->GetLives() <= 0) {
            new GameOver(this, "../Assets/Fonts/Arial.ttf", true);
            mShip1->SetState(ActorState::Destroy);
        }
        else if (mShip2 && mShip2->GetState() == ActorState::Active && mShip2->GetLives() <= 0) {
            new GameOver(this, "../Assets/Fonts/Arial.ttf", false);
            mShip2->SetState(ActorState::Destroy);
        }
    }

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

// Adiciona um ator à lista de atores (ou à lista pendente se estiver atualizando)
void Game::AddActor(Actor* actor)
{
    if (mUpdatingActors) {
        mPendingActors.emplace_back(actor);
    }else {
        mActors.emplace_back(actor);
    }
}

// Remove um ator das listas de atores
void Game::RemoveActor(Actor* actor)
{
    RemoveActorFromVector(mActors, actor);
    RemoveActorFromVector(mPendingActors, actor);
}

// Adiciona um componente desenhavel e ordena por ordem de desenho
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

// Remove um componente desenhavel da lista
void Game::RemoveDrawable(class DrawComponent *drawable)
{
    std::vector<class DrawComponent *>::iterator it;
    it = std::find(mDrawables.begin(), mDrawables.end(), drawable);
    if (it != mDrawables.end()) {
        mDrawables.erase(it);
    }
}

// Renderiza o frame atual: UI ou cena do jogo com grid e atores
void Game::GenerateOutput()
{
    mRenderer->BeginRenderToTexture();
    mRenderer->Clear();
    
    bool hasActiveUI = false;
    for (auto ui : mUIStack) {
        if (ui->GetState() == UIScreen::UIState::Active) {
            hasActiveUI = true;
            break;
        }
    }
    
    if (hasActiveUI) {
        for (auto it = mUIStack.rbegin(); it != mUIStack.rend(); ++it) {
            auto ui = *it;
            if (ui->GetState() == UIScreen::UIState::Active) {
                OpeningScreen* openingScreen = dynamic_cast<OpeningScreen*>(ui);
                if (openingScreen) {
                    openingScreen->Draw(mRenderer);
                } else {
                    mRenderer->Draw();
                }
                break;
            }
        }
    } else {
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
    }
    
    mRenderer->EndRenderToTexture();
    mRenderer->Present();
}

// Remove todos os atores e telas UI da cena atual
void Game::UnloadScene()
{
    for(auto *actor : mActors) {
        actor->SetState(ActorState::Destroy);
    }

    for (auto ui : mUIStack) {
        delete ui;
    }
    mUIStack.clear();
}

// Carrega uma nova cena do jogo, descarregando a anterior
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
            InitializeActors();
            break;
        }
    }
}

// Limpa todos os recursos do jogo antes de encerrar
void Game::Shutdown()
{
    while (!mActors.empty()) {
        delete mActors.back();
    }

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

// Remove um ator de um vetor usando swap com o último elemento para eficiência
void Game::RemoveActorFromVector(std::vector<class Actor*> &actors, class Actor *actor) {
    auto it = std::find(actors.begin(), actors.end(), actor);
    if (it != actors.end()) {
        std::iter_swap(it, actors.end() - 1);
        actors.pop_back();
    }
}

// Verifica colisões entre lasers ativos e as naves
void Game::CheckLaserCollisions()
{
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
}