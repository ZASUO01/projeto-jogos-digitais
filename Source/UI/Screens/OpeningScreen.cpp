#include "OpeningScreen.h"
#include "../../Game.h"
#include "../../Renderer/VideoPlayer.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/Shader.h"
#include "../../Renderer/VertexArray.h"
#include "../../Math.h"
#include "../UIImage.h"
#include <SDL.h>
#include <GL/glew.h>
#include <iostream>

OpeningScreen::OpeningScreen(class Game* game)
    : UIScreen(game, "../Assets/Fonts/Arial.ttf")
    , mVideoPlayer(nullptr)
    , mIsLooping(false)
    , mHasPlayedOnce(false)
    , mVideoVerts(nullptr)
{
    // Criar VideoPlayer
    mVideoPlayer = new VideoPlayer();
    
    // Carregar vídeo begin.mp4
    std::string videoPath = "../Opening/begin.mp4";
    if (!mVideoPlayer->Load(videoPath))
    {
        SDL_Log("Erro ao carregar vídeo: %s", videoPath.c_str());
        // Se não conseguir carregar, pular direto para o menu
        Close();
        GetGame()->SetScene(GameScene::MainMenu);
        return;
    }
    
    // Criar vertex array para renderizar o vídeo
    // Usar coordenadas normalizadas como no sprite shader (-0.5 a 0.5)
    float vertices[] = {
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top left
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top right
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f  // bottom left
    };
    
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};
    mVideoVerts = new VertexArray(vertices, 4, indices, 6);
}

OpeningScreen::~OpeningScreen()
{
    if (mVideoPlayer)
    {
        delete mVideoPlayer;
        mVideoPlayer = nullptr;
    }
    
    if (mVideoVerts)
    {
        delete mVideoVerts;
        mVideoVerts = nullptr;
    }
}

void OpeningScreen::Update(float deltaTime)
{
    if (!mVideoPlayer)
        return;
    
    // Verificar se o vídeo terminou antes de atualizar
    if (mVideoPlayer->IsFinished() && !mIsLooping)
    {
        // Vídeo terminou pela primeira vez, iniciar loop a partir dos 5 segundos
        mHasPlayedOnce = true;
        mIsLooping = true;
        double loopTime = 5.0;
        if (loopTime < mVideoPlayer->GetDuration())
        {
            mVideoPlayer->SeekToTime(loopTime);
            mVideoPlayer->ResetFinished();
        }
        return;
    }
    else if (mVideoPlayer->IsFinished() && mIsLooping)
    {
        // Loop terminou, voltar para os 5 segundos
        double loopTime = 5.0;
        if (loopTime < mVideoPlayer->GetDuration())
        {
            mVideoPlayer->SeekToTime(loopTime);
            mVideoPlayer->ResetFinished();
        }
        return;
    }
    
    // Atualizar vídeo
    mVideoPlayer->Update(deltaTime);
}

void OpeningScreen::HandleKeyPress(int key)
{
    // Se Enter for pressionado, ir para o menu principal
    if (key == SDLK_RETURN || key == SDLK_RETURN2)
    {
        Close();
        GetGame()->SetScene(GameScene::MainMenu);
    }
}

void OpeningScreen::Draw(class Renderer* renderer)
{
    if (!mVideoPlayer || !mVideoVerts)
        return;
    
    class Shader* spriteShader = renderer->GetSpriteShader();
    if (!spriteShader)
        return;
    
    spriteShader->SetActive();
    
    // Obter o tamanho real da janela
    SDL_Window* window = GetGame()->GetWindow();
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    
    float width = static_cast<float>(windowWidth);
    float height = static_cast<float>(windowHeight);
    
    // Configurar view projection (mesma usada para sprites)
    Matrix4 viewProj = Matrix4::CreateSimpleViewProj(width, height);
    spriteShader->SetMatrixUniform("uViewProj", viewProj);
    
    // Criar matriz de transformação para ajustar o vídeo à tela
    // Usar "fit" ao invés de "fill" - o vídeo deve caber inteiro na tela
    int videoWidth = mVideoPlayer->GetWidth();
    int videoHeight = mVideoPlayer->GetHeight();
    
    float scaleX = width / static_cast<float>(videoWidth);
    float scaleY = height / static_cast<float>(videoHeight);
    float scale = (scaleX < scaleY) ? scaleX : scaleY; // Usar o menor para caber inteiro (fit)
    
    float scaledWidth = static_cast<float>(videoWidth) * scale;
    float scaledHeight = static_cast<float>(videoHeight) * scale;
    
    // Os vertices estão em coordenadas normalizadas (-0.5 a 0.5), centro em (0,0)
    // Seguindo a mesma lógica do UIImage: scaleMat * rotMat * transMat
    // Mas como queremos centralizar, vamos usar apenas scale e translate para o centro
    // O CreateSimpleViewProj converte pixels para espaço de clip, então (width/2, height/2) é o centro
    Matrix4 scaleMat = Matrix4::CreateScale(scaledWidth, scaledHeight, 1.0f);
    Matrix4 transMat = Matrix4::CreateTranslation(Vector3(0.0f, 0.0f, 0.0f)); // Centro já está em (0,0)
    Matrix4 world = scaleMat * transMat; // Mesma ordem do UIImage
    
    spriteShader->SetMatrixUniform("uWorldTransform", world);
    
    // Configurar textura
    spriteShader->SetTextureUniform("uTexture", mVideoPlayer->GetTextureID(), 0);
    spriteShader->SetFloatUniform("uTextureFactor", 1.0f);
    spriteShader->SetVectorUniform("uBaseColor", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    
    // Desenhar usando nossos vertices
    mVideoVerts->SetActive();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

