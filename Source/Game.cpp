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
#include "Renderer/AudioPlayer.h"
#include "PathResolver.h"
#include <SDL_mixer.h>
#include "UI/Screens/Connect.h"
#include "UI/Screens/Endgame.h"


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
        ,mBackgroundAudio(nullptr)
        ,mClient(nullptr)
        ,inMultiplayer(false)
        ,mNetTicksCount(0)
        ,mPlayer(nullptr)
        ,mIsPlayerSet(false)
{}

// Inicializa o jogo, criando a janela SDL, o renderer e a tela de abertura
bool Game::Initialize()
{
    Random::Init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }
    
    // Inicializar SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_Log("Erro ao inicializar SDL_mixer: %s", Mix_GetError());
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

    mClient = new Client(this);
    mClient->Initialize();

    new OpeningScreen(this);

    mTicksCount = SDL_GetTicks();
    mNetTicksCount = mTicksCount;

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
    while (mIsRunning){
        ProcessInput();
        UpdateGame();
        GenerateOutput();
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
                        new GameOver(this, PathResolver::ResolvePath("Assets/Fonts/Arial.ttf"), true);
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

    if (inMultiplayer) {
        mClient->AddInput(state);

        if (mIsPlayerSet) {
            mPlayer->ProcessInput(state);
        }
    }else {
        unsigned int size = mActors.size();
        for (unsigned int i = 0; i < size; ++i) {
            mActors[i]->ProcessInput(state);
        }
    }
}

// Atualiza a lógica do jogo: atores, UI e remove elementos fechados
void Game::UpdateGame()
{
    if (inMultiplayer) {
        if (mIsPlayerSet) {
            mPlayer->Update(SIM_DELTA_TIME);
        }
        UpdateLocalActors(SIM_DELTA_TIME);

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
    }else {
        UpdateActors(SIM_DELTA_TIME);
    }
    for (auto ui : mUIStack) {
        if (ui->GetState() == UIScreen::UIState::Active) {
            ui->Update(SIM_DELTA_TIME);
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

    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}
    mTicksCount = SDL_GetTicks();
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
            new GameOver(this, PathResolver::ResolvePath("Assets/Fonts/Arial.ttf"), true);
            mShip1->SetState(ActorState::Destroy);
        }
        else if (mShip2 && mShip2->GetState() == ActorState::Active && mShip2->GetLives() <= 0) {
            new GameOver(this, PathResolver::ResolvePath("Assets/Fonts/Arial.ttf"), false);
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
            // Parar áudio de fundo quando voltar ao menu
            if (mBackgroundAudio)
            {
                mBackgroundAudio->Stop();
            }
            new MainMenu(this, PathResolver::ResolvePath("Assets/Fonts/Arial.ttf"));
            inMultiplayer = false;
            break;
        }
        case GameScene::Connect: {
            new Connect(this, PathResolver::ResolvePath("Assets/Fonts/Arial.ttf"));
            inMultiplayer = false;
            break;
        }
        case GameScene::End: {
            new EndGame(this, PathResolver::ResolvePath("Assets/Fonts/Arial.ttf"));
            break;
        }
        case GameScene::Multiplayer: {
            ResetBackgroundAudio();
            new Floor(this);
            inMultiplayer = true;
            break;
        }
        case GameScene::Level1:
        {
            ResetBackgroundAudio();
            InitializeActors();
            inMultiplayer = false;
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

    // Parar e liberar áudio de fundo
    if (mBackgroundAudio)
    {
        mBackgroundAudio->Stop();
        delete mBackgroundAudio;
        mBackgroundAudio = nullptr;
    }

    mRenderer->Shutdown();
    delete mRenderer;
    mRenderer = nullptr;

    mClient->Disconnect();
    mClient->Shutdown();
    delete mClient;
    mClient = nullptr;

    Mix_CloseAudio();
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
                    // Verificar se este laser já atingiu esta nave
                    if (!laserComp->HasHitShip(mShip1)) {
                        CircleColliderComponent* collider = mShip1->GetComponent<CircleColliderComponent>();
                        if (collider) {
                            if (laserComp->IntersectCircle(mShip1->GetPosition(), collider->GetRadius())) {
                                mShip1->TakeDamage();
                                laserComp->MarkShipHit(mShip1);
                            }
                        }
                    }
                }
                
                if (mShip2 && mShip2->GetState() == ActorState::Active && mShip2 != ownerShip && !mShip2->IsInvincible()) {
                    // Verificar se este laser já atingiu esta nave
                    if (!laserComp->HasHitShip(mShip2)) {
                        CircleColliderComponent* collider = mShip2->GetComponent<CircleColliderComponent>();
                        if (collider) {
                            if (laserComp->IntersectCircle(mShip2->GetPosition(), collider->GetRadius())) {
                                mShip2->TakeDamage();
                                laserComp->MarkShipHit(mShip2);
                            }
                        }
                    }
                }
            }
        }
    }
}


void Game::ResetBackgroundAudio() {
    // Parar e descarregar áudio anterior se existir (para reiniciar do zero)
    if (mBackgroundAudio)
    {
        mBackgroundAudio->Stop();
        mBackgroundAudio->Unload();
    }

    // Criar ou recriar áudio de fundo quando o jogo começar/reiniciar
    if (!mBackgroundAudio)
    {
        mBackgroundAudio = new AudioPlayer();
    }

    std::string audioPath = PathResolver::ResolvePath("Opening/abertura.wav");
    if (!mBackgroundAudio->Load(audioPath))
    {
        SDL_Log("Erro ao carregar áudio de fundo: %s", audioPath.c_str());
        delete mBackgroundAudio;
        mBackgroundAudio = nullptr;
    }
    else
    {
        mBackgroundAudio->SetVolume(11);
        mBackgroundAudio->Play(true);
    }
}

void Game::SetPlayer(const Vector2 &position, const float rotation) {
    if (mIsPlayerSet) {
        return;
    }

    mPlayer = new Ship(this, 40, 300, 3, Vector3(0.0f, 0.7f, 0.7f), false);
    mPlayer->SetType(ActorType::Network);
    mPlayer->SetPosition(position);
    mPlayer->SetRotation(rotation);
    mIsPlayerSet = true;
}

void Game::SetPlayerState(const RawState& raw) const {
    const auto newPlayerPos = Vector2(raw.posX, raw.posY);

    mPlayer->SetPosition(newPlayerPos);
    mPlayer->SetRotation(raw.rotation);
    mPlayer->SetLives(raw.life);
    mPlayer->SetInvincibilityTimer(raw.invulnerableTimer);
}

bool Game::IsEnemySet(const int id) {
    if (mEnemies.find(id) == mEnemies.end()) {
        return false;
    }
    return true;
}

void Game::SetEnemy(const int id,const Vector2 &position, const float rotation) {
        auto enemy = new Ship(
            this,
            40,
            300,
            3,
            Vector3(1.0f, 0.0f, 0.0f),
            true);
        enemy->SetPosition(position);
        enemy->SetRotation(rotation);
        enemy->SetType(ActorType::Local);
        mEnemies.emplace(id, enemy);
        mEnemiesLastUpdate.emplace(id, std::chrono::steady_clock::now());

        mEnemiesTargets[id] = {position.x, position.y, rotation};
}


void Game::SetEnemiesState(const std::vector<OtherState> &others)  {
    for (const auto& other : others) {
        if (mEnemiesTargets.find(other.id) == mEnemiesTargets.end()) {
            mEnemiesTargets[other.id] = {other.posX, other.posY, other.rotation};
        } else {
            mEnemiesTargets[other.id].targetPosX = other.posX;
            mEnemiesTargets[other.id].targetPosY = other.posY;
            mEnemiesTargets[other.id].targetRotation = other.rotation;

            if (other.hasShot) {
                const auto &enemy = mEnemies[other.id];
                const auto lb = new LaserBeam(
                    this,
                    enemy->GetPosition(),
                    enemy->GetRotation(),
                    Vector3(1, 0, 1),
                    enemy);

                lb->SetType(ActorType::Local);
            }

            mEnemies[other.id]->SetLives(other.life);
            mEnemies[other.id]->SetInvincibilityTimer(other.invulnerableTimer);
        }

        if (auto it1 = mEnemiesLastUpdate.find(other.id); it1 != mEnemiesLastUpdate.end()) {
            it1->second = std::chrono::steady_clock::now();
        }
    }
}

void Game::InterpolateEnemies() {
    for (auto& [id, enemy] : mEnemies) {
        if (mEnemiesTargets.find(id) == mEnemiesTargets.end()) {
            continue;
        }

        const auto&[targetPosX, targetPosY, targetRotation] = mEnemiesTargets[id];
        const Vector2 currentPos = enemy->GetPosition();
        const float currentRot = enemy->GetRotation();

        // Position interpolation (Lerp)
        // curent + (target - current) * fator
        const float diffX = targetPosX - currentPos.x;
        const float diffY = targetPosY - currentPos.y;

        const float newPosX = currentPos.x + (diffX * INTERPOLATION_FACTOR);
        const float newPosY = currentPos.y + (diffY * INTERPOLATION_FACTOR);

        const auto newPos = Vector2(newPosX, newPosY);
        enemy->SetPosition(newPos);

        // Rotation interpolation
        const float rotDiff = targetRotation - currentRot;
        const float newRot = currentRot + (rotDiff * INTERPOLATION_FACTOR);
        enemy->SetRotation(newRot);
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

void Game::UpdateLocalActors(const float deltaTime) const {
    for (const auto& actor : mActors) {
        if (actor->GetType() == ActorType::Local) {
            actor->Update(deltaTime);
        }
    }
}