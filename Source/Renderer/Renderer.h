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
#include "Texture.h"
#include "Font.h"

class Renderer
{
public:
    Renderer(SDL_Window* window);
    ~Renderer();

    bool Initialize(float width, float height);
    void UpdateScreenSize(float width, float height);
    void Shutdown();
    void UnloadData();

    void Clear();
    void Draw(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color);
    void DrawFilled(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color);
    void DrawFilledWithAlpha(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color, float alpha);
    
    // Desenha o grid isométrico neon como fundo
    void DrawAdvancedGrid(float screenWidth, float screenHeight, float time);

    void Present();

    void AddUIElement(class UIElement *comp);
    void RemoveUIElement(class UIElement *comp);

    // Getters
    class Shader* GetBaseShader() const { return mBaseShader; }
    float GetScreenWidth() const { return mScreenWidth; }
    float GetScreenHeight() const { return mScreenHeight; }
    class Shader* GetSpriteShader() const { return mSpriteShader; }
    class Texture* GetTexture(const std::string& fileName);
    class Font* GetFont(const std::string& fileName);

private:
    bool LoadShaders();
    void CreateSpriteVerts();
    std::string FindShaderPath(const std::string& shaderName);

    // Game
    class Game* mGame;

    // Sprite shader for UI
    class Shader* mSpriteShader;
    // Base shader for game
    class Shader* mBaseShader;
    
    // Advanced Grid shader e recursos
    class Shader* mAdvancedGridShader;
    class VertexArray* mFullScreenQuad;
    float mScreenWidth;
    float mScreenHeight;

    // Sprite vertex array
    class VertexArray *mSpriteVerts;

    // Window
    SDL_Window* mWindow;

    // OpenGL context
    SDL_GLContext mContext;

    // Ortho projection for 2D shaders
    Matrix4 mOrthoProjection;

    // Map of textures loaded
    std::unordered_map<std::string, class Texture*> mTextures;
    // Map of fonts loaded
    std::unordered_map<std::string, class Font*> mFonts;

    // UI screens to draw
    std::vector<class UIElement*> mUIComps;

    // Width/height of screen
    float mScreenWidth;
    float mScreenHeight;
};