#pragma once
#include <SDL.h>
#include <vector>

#include "Actors/Ship.h"
#include "Actors/Actor.h"
#include "Renderer/Renderer.h"

enum class GameScene
{
    MainMenu,
    Level1
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

private:
    void ProcessInput();
    void UpdateGame(float deltaTime);
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
};
