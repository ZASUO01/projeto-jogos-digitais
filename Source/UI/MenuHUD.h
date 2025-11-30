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

    // Valores proporcionais à resolução base (1920x1080)
    static const float START_Y_RATIO;      // 400/1080 ≈ 0.370
    static const float CONNECT_Y_RATIO;     // 500/1080 ≈ 0.463
    static const float ARROW_X_OFFSET_RATIO; // 200/1920 ≈ 0.104
    static const float ARROW_SIZE_RATIO;    // 30/1080 ≈ 0.028
};

