//
// Created by Lucas N. Ferreira on 06/11/25.
//

#include "MainMenu.h"
#include "../../Game.h"
#include "../../Math.h"
#include <SDL.h>

MainMenu::MainMenu(class Game* game, const std::string& fontName)
        :UIScreen(game, fontName)
{
	UIButton* startButton = AddButton("Iniciar Jogo", [this]() {
		Close();
		GetGame()->SetScene(GameScene::Level1);
	}, Vector2(0.0f, -50.0f));

	startButton->SetTextColor(Color::White);
	startButton->SetBackgroundColor(Vector4(0.0f, 0.0f, 1.0f, 1.0f));

	UIButton* quitButton = AddButton("Fechar Jogo", [this]() {
		GetGame()->Quit();
	}, Vector2(0.0f, -150.0f));

	quitButton->SetTextColor(Color::White);
	quitButton->SetBackgroundColor(Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}

void MainMenu::HandleKeyPress(int key)
{
	if (mButtons.empty() || mSelectedButtonIndex < 0)
		return;

	if (key == SDLK_UP)
	{
		mButtons[mSelectedButtonIndex]->SetHighlighted(false);

		mSelectedButtonIndex--;
		if (mSelectedButtonIndex < 0)
		{
			mSelectedButtonIndex = static_cast<int>(mButtons.size()) - 1;
		}

		mButtons[mSelectedButtonIndex]->SetHighlighted(true);
	}
	else if (key == SDLK_DOWN)
	{
		mButtons[mSelectedButtonIndex]->SetHighlighted(false);

		mSelectedButtonIndex++;
		if (mSelectedButtonIndex >= static_cast<int>(mButtons.size()))
		{
			mSelectedButtonIndex = 0;
		}

		mButtons[mSelectedButtonIndex]->SetHighlighted(true);
	}
	else if (key == SDLK_RETURN || key == SDLK_RETURN2)
	{
		mButtons[mSelectedButtonIndex]->OnClick();
	}
}