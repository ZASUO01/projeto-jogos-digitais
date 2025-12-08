#pragma once
#include <GL/glew.h>
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
    void Draw();
    void Draw(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color);
    void DrawFilled(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color);
    void DrawFilledWithAlpha(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color, float alpha);
    void DrawWithAlpha(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color, float alpha);
    
    void DrawAdvancedGrid(float screenWidth, float screenHeight, float time);

    void BeginRenderToTexture();
    void EndRenderToTexture();
    
    void Present();

    void AddUIElement(class UIElement *comp);
    void RemoveUIElement(class UIElement *comp);

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

    class Game* mGame;

    class Shader* mSpriteShader;
    class Shader* mBaseShader;
    
    class Shader* mAdvancedGridShader;
    class VertexArray* mFullScreenQuad;
    
    class Shader* mCRTShader;
    
    GLuint mFBO;
    GLuint mSceneTexture;
    GLuint mRBO;
    
    float mScreenWidth;
    float mScreenHeight;

    class VertexArray *mSpriteVerts;

    SDL_Window* mWindow;

    SDL_GLContext mContext;

    Matrix4 mOrthoProjection;

    std::unordered_map<std::string, class Texture*> mTextures;
    std::unordered_map<std::string, class Font*> mFonts;

    std::vector<class UIElement*> mUIComps;
};