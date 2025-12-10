#pragma once
#include <SDL.h>
#include <vector>

#include "Actors/Ship.h"
#include "Actors/Actor.h"
#include "Renderer/Renderer.h"
#include  "../Client/Client.h"
#include <map>
#include <chrono>

enum class GameScene
{
    MainMenu,
    Connect,
    End,
    Level1,
    Multiplayer
};

class Game{
public:
    Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    void InitializeActors();
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);

    void PushUI(class UIScreen* screen) { mUIStack.emplace_back(screen); }
    const std::vector<class UIScreen*>& GetUIStack() { return mUIStack; }

    class Renderer* GetRenderer() { return mRenderer; }

    SDL_Window* GetWindow() { return mWindow; }
    int GetWindowWidth() const;
    int GetWindowHeight() const;

    static const int WINDOW_WIDTH = 1920;
    static const int WINDOW_HEIGHT = 1080;
    static const int RENDER_WIDTH = 1920;
    static const int RENDER_HEIGHT = 1080;
    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);

    std::vector<class DrawComponent*>& GetDrawables() { return mDrawables; }

    Ship *GetShip() const {return mShip; }
    Ship *GetShip1() const {return mShip1; }
    Ship *GetShip2() const {return mShip2; }

    void SetScene(GameScene scene);
    void UnloadScene();

    // Networking stuff
    [[nodiscard]] bool IsMultiplayer() const { return inMultiplayer; }
    [[nodiscard]] class Client* GetClient() const { return mClient; }
    static constexpr float SIM_DELTA_TIME = 1.0f / 60.0f;
    [[nodiscard]] bool IsPlayerSet() const { return mIsPlayerSet; }
    void SetPlayer(const Vector2 &position, float rotation);
    [[nodiscard]] Ship* GetPlayer() const { return mPlayer; }
    void SetPlayerState(const RawState& raw) const;
    bool IsEnemySet(int id);
    void SetEnemy(int id, const Vector2 &position, float rotation);
    void SetEnemiesState(const std::vector<OtherState> &others);

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    void RemoveActorFromVector(std::vector< Actor*> &actors,  Actor *actor);
    void CheckLaserCollisions();

    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;
    std::vector<class DrawComponent*> mDrawables;

    std::vector<class UIScreen*> mUIStack;

    SDL_Window* mWindow;
    class Renderer* mRenderer;
    Uint32 mTicksCount;
    bool mIsRunning;
    bool mIsDebugging;
    bool mUpdatingActors;
    
    Ship* mShip;
    Ship* mShip1;
    Ship* mShip2;
    
    class AudioPlayer* mBackgroundAudio;
    void ResetBackgroundAudio();

    // Networking stuff
    class Client* mClient;
    bool inMultiplayer;
    Uint32 mNetTicksCount;
    Ship* mPlayer;
    bool mIsPlayerSet;
    std::map<int, Ship*> mEnemies;
    std::map<int, std::chrono::steady_clock::time_point> mEnemiesLastUpdate;
    std::map<int, InterpolationState> mEnemiesTargets;
    static constexpr int ENEMY_RESPONSE_TIMEOUT_MS = 500;
    static constexpr float INTERPOLATION_FACTOR = 0.2f;
    void UpdateLocalActors(float deltaTime) const;
    void InterpolateEnemies();
    void RemoveInactiveEnemies();
};
