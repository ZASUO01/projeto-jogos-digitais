//
// Created by Lucas N. Ferreira on 06/11/25.
//

#include "Endgame.h"
#include "../../Game.h"
#include "../../Math.h"
#include "../UITriangle.h"
#include <SDL.h>

EndGame::EndGame(class Game* game, const std::string& fontName)
        :UIScreen(game, fontName)
        , mLeftArrow(nullptr)
        , mRightArrow(nullptr)
{
	UIRect* bgRect = AddRect(Vector2(0.0f, 0.0f), Vector2(1024.0f, 768.0f), 1.0f, 0.0f, -100);
	bgRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f)); // Solid black background

	// Add winner ship text
	UIText* winnerTextElement = AddText("Game Over", Vector2(0.0f, 100.0f), 1.5f, 0.0f, 60, 1024, 100);
	winnerTextElement->SetTextColor(Vector3(1.0f, 0.0f, 0.0f));

	UIButton* restartButton = AddButton("Back to menu", [this]() {
		Close();
		GetGame()->SetScene(GameScene::MainMenu);
	}, Vector2(0.0f, -50.0f));

	restartButton->SetTextColor(Color::White);
	restartButton->SetBackgroundColor(Vector4(0.0f, 0.0f, 1.0f, 1.0f));

	UIButton* menuButton = AddButton("Exit", [this]() {
		Close();
		GetGame()->Quit();
	}, Vector2(0.0f, -150.0f));

	menuButton->SetTextColor(Color::White);
	menuButton->SetBackgroundColor(Vector4(0.0f, 0.0f, 1.0f, 1.0f));


	mLeftArrow = AddTriangle(Vector2(-200.0f, -50.0f), 30.0f, -Math::Pi / 2.0f, 200);
	mLeftArrow->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f)); // Inicialmente invisível
	mLeftArrow->SetIsVisible(false);


	mRightArrow = AddTriangle(Vector2(200.0f, -50.0f), 30.0f, Math::Pi / 2.0f, 200);
	mRightArrow->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f)); // Inicialmente invisível
	mRightArrow->SetIsVisible(false);

	UpdateArrows();
}

void EndGame::HandleKeyPress(const int key)
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
		UpdateArrows();
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
		UpdateArrows();
	}
	else if (key == SDLK_RETURN || key == SDLK_RETURN2)
	{
		mButtons[mSelectedButtonIndex]->OnClick();
	}
}

void EndGame::UpdateArrows()
{
	// Esconder todas as setas primeiro
	if (mLeftArrow)
	{
		mLeftArrow->SetIsVisible(false);
	}
	if (mRightArrow)
	{
		mRightArrow->SetIsVisible(false);
	}

	if (!mLeftArrow || !mRightArrow || mButtons.empty() || mSelectedButtonIndex < 0)
		return;

	// Obter o botão selecionado
	UIButton* selectedButton = mButtons[mSelectedButtonIndex];
	if (!selectedButton)
		return;

	// Obter a posição do botão selecionado
	Vector2 buttonPos = selectedButton->GetOffset();

	// Calcular largura aproximada do botão (baseado no texto)
	float buttonWidth = 200.0f; // Estimativa da largura do botão
	float arrowDistance = buttonWidth / 2.0f + 60.0f; // Distância das setas do botão

	// Posicionar setas ao lado do botão selecionado
	// Seta esquerda: à esquerda do botão, apontando para direita (>)
	mLeftArrow->SetOffset(Vector2(buttonPos.x - arrowDistance, buttonPos.y));
	mLeftArrow->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)); // Branco, visível
	mLeftArrow->SetIsVisible(true);

	// Seta direita: à direita do botão, apontando para esquerda (<)
	mRightArrow->SetOffset(Vector2(buttonPos.x + arrowDistance, buttonPos.y));
	mRightArrow->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)); // Branco, visível
	mRightArrow->SetIsVisible(true);
}