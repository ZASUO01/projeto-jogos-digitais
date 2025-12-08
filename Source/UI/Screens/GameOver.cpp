//
// Created by Lucas N. Ferreira on 06/11/25.
//

#include "GameOver.h"
#include "../../Game.h"
#include "../../Math.h"
#include "../UITriangle.h"
#include <SDL.h>

GameOver::GameOver(class Game* game, const std::string& fontName, bool isRedShipWinner)
        :UIScreen(game, fontName)
        , mLeftArrow(nullptr)
        , mRightArrow(nullptr)
{
	// Add black background rectangle covering the entire screen
	// Screen coordinates: center is (0,0), window is 1024x768
	// Need to cover from -512 to +512 in x and -384 to +384 in y
	UIRect* bgRect = AddRect(Vector2(0.0f, 0.0f), Vector2(1024.0f, 768.0f), 1.0f, 0.0f, -100);
	bgRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f)); // Solid black background
	
	// Determinar cor da nave vencedora
	Vector3 winnerColor = isRedShipWinner ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 0.7f, 0.7f);
	std::string winnerText = isRedShipWinner ? "Vermelho Venceu" : "Azul Venceu";
	
	// Add winner ship text
	UIText* winnerTextElement = AddText(winnerText, Vector2(0.0f, 100.0f), 1.5f, 0.0f, 60, 1024, 100);
	winnerTextElement->SetTextColor(winnerColor);
	
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
	
	// Criar setas triangulares (inicialmente invisíveis, serão atualizadas quando houver seleção)
	// Seta esquerda: triângulo apontando para direita (>)
	mLeftArrow = AddTriangle(Vector2(-200.0f, -50.0f), 30.0f, -Math::Pi / 2.0f, 200);
	mLeftArrow->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f)); // Inicialmente invisível
	mLeftArrow->SetIsVisible(false);
	
	// Seta direita: triângulo apontando para esquerda (<)
	mRightArrow = AddTriangle(Vector2(200.0f, -50.0f), 30.0f, Math::Pi / 2.0f, 200);
	mRightArrow->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f)); // Inicialmente invisível
	mRightArrow->SetIsVisible(false);
	
	// Atualizar setas para o botão selecionado inicial
	UpdateArrows();
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

void GameOver::UpdateArrows()
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