//
// Created by Lucas N. Ferreira on 06/11/25.
//

#include "MainMenu.h"
#include "../../Game.h"
#include "../../Math.h"
#include "../../Renderer/VideoPlayer.h"
#include "../../Renderer/AudioPlayer.h"
#include "../../PathResolver.h"
#include "../UIVideo.h"
#include "../UITriangle.h"
#include <SDL.h>
#include <cmath>

MainMenu::MainMenu(class Game* game, const std::string& fontName)
        :UIScreen(game, fontName)
        , mVideoPlayer(nullptr)
        , mAudioPlayer(nullptr)
        , mVideoImage(nullptr)
        , mLeftArrow(nullptr)
        , mRightArrow(nullptr)
{
    // Criar VideoPlayer para o background
    mVideoPlayer = new VideoPlayer();
    
    // Carregar vídeo abertura.mp4
    std::string videoPath = PathResolver::ResolvePath("Opening/abertura.mp4");
    if (!mVideoPlayer->Load(videoPath))
    {
        SDL_Log("Erro ao carregar vídeo de background: %s", videoPath.c_str());
        // Se não conseguir carregar, continuar sem o vídeo
    }
    else
    {
        // Criar UIVideo que cobre toda a tela como background
        // Calcular escala para preencher a tela (fill) - usar o maior para preencher completamente
        int windowWidth = GetGame()->GetWindowWidth();
        int windowHeight = GetGame()->GetWindowHeight();
        int videoWidth = mVideoPlayer->GetWidth();
        int videoHeight = mVideoPlayer->GetHeight();
        
        // Calcular escala para preencher toda a tela (fill)
        // Usar o maior scale para garantir que preencha toda a tela
        float scaleX = static_cast<float>(windowWidth) / static_cast<float>(videoWidth);
        float scaleY = static_cast<float>(windowHeight) / static_cast<float>(videoHeight);
        float scale = (scaleX > scaleY) ? scaleX : scaleY; // Fill - usar o maior para preencher
        
        // Criar imagem de vídeo centralizada que preenche toda a tela
        mVideoImage = new UIVideo(GetGame(), mVideoPlayer, Vector2(0.0f, 0.0f), scale, 0.0f, 0);
    }
    
    // Criar AudioPlayer
    mAudioPlayer = new AudioPlayer();
    
    // Carregar áudio abertura.wav
    std::string audioPath = PathResolver::ResolvePath("Opening/abertura.wav");
    if (!mAudioPlayer->Load(audioPath))
    {
        SDL_Log("Erro ao carregar áudio de background: %s", audioPath.c_str());
        // Continuar mesmo sem áudio
    }
    else
    {
        // Tocar áudio em loop junto com o vídeo
        mAudioPlayer->Play(true);
    }

	UIButton* startButton = AddButton("Iniciar Jogo", [this]() {
		Close();
		GetGame()->SetScene(GameScene::Level1);
	}, Vector2(0.0f, -80.0f));

	// Tornar botão transparente (background e texto totalmente transparentes)
	startButton->SetTextColor(Color::White);
	startButton->SetBackgroundColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f)); // Background transparente
	startButton->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f)); // Texto totalmente transparente

	UIButton* quitButton = AddButton("Fechar Jogo", [this]() {
		GetGame()->Quit();
	}, Vector2(0.0f, -280.0f));

	// Tornar botão transparente (background e texto totalmente transparentes)
	quitButton->SetTextColor(Color::White);
	quitButton->SetBackgroundColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f)); // Background transparente
	quitButton->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f)); // Texto totalmente transparente
	
	// Criar setas triangulares (inicialmente invisíveis, serão atualizadas quando houver seleção)
	// Setas apontando para o botão (esquerda aponta para direita, direita aponta para esquerda)
	
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

MainMenu::~MainMenu()
{
    if (mAudioPlayer)
    {
        mAudioPlayer->Stop();
        delete mAudioPlayer;
        mAudioPlayer = nullptr;
    }
    
    if (mVideoImage)
    {
        delete mVideoImage;
        mVideoImage = nullptr;
    }
    
    if (mVideoPlayer)
    {
        delete mVideoPlayer;
        mVideoPlayer = nullptr;
    }
}

void MainMenu::Update(float deltaTime)
{
    if (mVideoPlayer)
    {
        // Atualizar vídeo
        bool stillPlaying = mVideoPlayer->Update(deltaTime);
        
        // Se o vídeo terminou, fazer loop
        if (!stillPlaying)
        {
            mVideoPlayer->SeekToTime(0.0);
            mVideoPlayer->ResetFinished();
            
            // Reiniciar áudio para sincronizar com o loop do vídeo
            if (mAudioPlayer)
            {
                mAudioPlayer->Stop();
                mAudioPlayer->Play(true);
            }
        }
    }
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

void MainMenu::UpdateArrows()
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
	// Vamos usar uma estimativa baseada no tamanho do texto
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
