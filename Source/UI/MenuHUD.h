#pragma once
#include <SDL.h>
#include <string>

class MenuHUD
{
public:
    enum class MenuState
    {
        Hidden,
        Menu,
        Connect
    };

    MenuHUD();
    ~MenuHUD();

    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer);
    void HandleKeyPress(SDL_Keycode key);
    void HandleTextInput(const char* text);

    MenuState GetState() const { return mState; }
    void SetState(MenuState state) { mState = state; }
    bool IsVisible() const { return mState != MenuState::Hidden; }
    
    bool WasStartSelected() const { return mStartSelected; }
    void ResetStartSelected() { mStartSelected = false; }
    void Show();

private:
    void RenderMenu(SDL_Renderer* renderer);
    void RenderConnectScreen(SDL_Renderer* renderer);
    void RenderArrow(SDL_Renderer* renderer, int x, int y, int size);

    MenuState mState;
    int mSelectedOption;
    std::string mInputText;
    bool mTextInputActive;
    bool mStartSelected;

    static const int START_Y = 400;
    static const int CONNECT_Y = 500;
    static const int ARROW_X_OFFSET = 200;
    static const int ARROW_SIZE = 30;
};

