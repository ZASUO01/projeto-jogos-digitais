#include "MenuHUD.h"

MenuHUD::MenuHUD()
    : mState(MenuState::Hidden)
    , mSelectedOption(0)
    , mTextInputActive(false)
    , mStartSelected(false)
{
}

MenuHUD::~MenuHUD()
{
}

void MenuHUD::Show()
{
    mState = MenuState::Menu;
    mSelectedOption = 0;
    mTextInputActive = false;
}

void MenuHUD::Update(float deltaTime)
{
}

void MenuHUD::Render(SDL_Renderer* renderer)
{
    if (mState == MenuState::Hidden)
        return;

    if (mState == MenuState::Menu)
    {
        RenderMenu(renderer);
    }
    else if (mState == MenuState::Connect)
    {
        RenderConnectScreen(renderer);
    }
}

void MenuHUD::RenderMenu(SDL_Renderer* renderer)
{
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    int arrowY = mSelectedOption == 0 ? START_Y + 20 : CONNECT_Y + 20;
    int arrowX = windowWidth / 2 - ARROW_X_OFFSET;
    RenderArrow(renderer, arrowX, arrowY, ARROW_SIZE);
}

void MenuHUD::RenderConnectScreen(SDL_Renderer* renderer)
{
}

void MenuHUD::RenderArrow(SDL_Renderer* renderer, int x, int y, int size)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    SDL_Point points[3];
    points[0] = {x, y - size/2};
    points[1] = {x + size, y};
    points[2] = {x, y + size/2};
    
    SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[1].x, points[1].y);
    SDL_RenderDrawLine(renderer, points[1].x, points[1].y, points[2].x, points[2].y);
    SDL_RenderDrawLine(renderer, points[2].x, points[2].y, points[0].x, points[0].y);
    
    int halfSize = size / 2;
    for (int i = -halfSize + 1; i < halfSize; i++) {
        int currentY = y + i;
        if (i < 0) {
            int width = (halfSize + i) * size / halfSize;
            SDL_RenderDrawLine(renderer, x, currentY, x + width, currentY);
        } else {
            int width = (halfSize - i) * size / halfSize;
            SDL_RenderDrawLine(renderer, x, currentY, x + width, currentY);
        }
    }
}

void MenuHUD::HandleKeyPress(SDL_Keycode key)
{
    if (mState == MenuState::Menu)
    {
        if (key == SDLK_UP || key == SDLK_w)
        {
            mSelectedOption = 0;
        }
        else if (key == SDLK_DOWN || key == SDLK_s)
        {
            mSelectedOption = 1;
        }
        else if (key == SDLK_RETURN || key == SDLK_RETURN2)
        {
            if (mSelectedOption == 0)
            {
                mStartSelected = true;
            }
            else if (mSelectedOption == 1)
            {
                mState = MenuState::Connect;
                mInputText.clear();
                mTextInputActive = true;
                SDL_StartTextInput();
            }
        }
    }
    else if (mState == MenuState::Connect)
    {
        if (key == SDLK_BACKSPACE)
        {
            if (!mInputText.empty())
            {
                mInputText.pop_back();
            }
            else
            {
                mState = MenuState::Menu;
                mTextInputActive = false;
                SDL_StopTextInput();
            }
        }
        else if (key == SDLK_ESCAPE)
        {
            mState = MenuState::Menu;
            mInputText.clear();
            mTextInputActive = false;
            SDL_StopTextInput();
        }
    }
}

void MenuHUD::HandleTextInput(const char* text)
{
    if (mState == MenuState::Connect && mTextInputActive)
    {
        mInputText += text;
    }
}

