//
// Created by pedro-souza on 23/11/2025.
//

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL.h>
#include "../Math.h"
#include "VertexArray.h"

class Renderer
{
public:
    Renderer(SDL_Window* window);
    ~Renderer();

    bool Initialize(float width, float height);
    void Shutdown();

    void Clear();
    void Draw(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color);

    void Present();

    // Getters
    class Shader* GetBaseShader() const { return mBaseShader; }

private:
    bool LoadShaders();

    // Game
    class Game* mGame;

    // Sprite shader
    class Shader* mBaseShader;

    // Window
    SDL_Window* mWindow;

    // OpenGL context
    SDL_GLContext mContext;

    // Ortho projection for 2D shaders
    Matrix4 mOrthoProjection;
};