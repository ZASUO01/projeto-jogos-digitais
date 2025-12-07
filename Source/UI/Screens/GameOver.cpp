//
// Created by Lucas N. Ferreira on 06/11/25.
//

#include "GameOver.h"
#include "../../Game.h"
#include "../../Math.h"
#include <SDL.h>

GameOver::GameOver(class Game* game, const std::string& fontName)
        :UIScreen(game, fontName)
{
	// Add black background rectangle covering the entire screen
	// Screen coordinates: center is (0,0), window is 1024x768
	// Need to cover from -512 to +512 in x and -384 to +384 in y
	UIRect* bgRect = AddRect(Vector2(0.0f, 0.0f), Vector2(1024.0f, 768.0f), 1.0f, 0.0f, -100);
	bgRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f)); // Solid black background
	
	// Add "Game Over" text
	UIText* gameOverText = AddText("Game Over", Vector2(0.0f, 100.0f), 1.5f, 0.0f, 60, 1024, 100);
	gameOverText->SetTextColor(Color::White);
	
	// Add "Recomeçar Jogo" button that restarts Level1
	UIButton* restartButton = AddButton("Recomeçar Jogo", [this]() {
		Close();
		GetGame()->SetScene(GameScene::Level1);
	}, Vector2(0.0f, -50.0f));
	
	restartButton->SetTextColor(Color::White);
	restartButton->SetBackgroundColor(Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	
	// Add "Voltar ao Menu" button that goes back to main menu
	UIButton* menuButton = AddButton("Voltar ao Menu", [this]() {
		Close();
		GetGame()->SetScene(GameScene::MainMenu);
	}, Vector2(0.0f, -150.0f));
	
	menuButton->SetTextColor(Color::White);
	menuButton->SetBackgroundColor(Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}

void GameOver::HandleKeyPress(int key)
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