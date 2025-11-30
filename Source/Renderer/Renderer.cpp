#include <GL/glew.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Font.h"
#include "../UI/UIElement.h"
#include <algorithm>
#include <SDL_image.h>
#include <SDL_ttf.h>

Renderer::Renderer(SDL_Window *window)
: mSpriteShader(nullptr)
, mBaseShader(nullptr)
, mAdvancedGridShader(nullptr)
, mFullScreenQuad(nullptr)
, mScreenWidth(0.0f)
, mScreenHeight(0.0f)
, mSpriteVerts(nullptr)
, mWindow(window)
, mContext(nullptr)
, mScreenWidth(1024.0f)
, mScreenHeight(768.0f)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(float width, float height)
{
    mScreenWidth = width;
    mScreenHeight = height;

    // Specify version 3.3 (core profile)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Enable double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Force OpenGL to use hardware acceleration
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Turn on vsync
    SDL_GL_SetSwapInterval(1);

    // Create an OpenGL context
    mContext = SDL_GL_CreateContext(mWindow);
    if (!mContext) {
        SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
        return false;
    }

    // Make sure the context is current
    if (SDL_GL_MakeCurrent(mWindow, mContext) != 0) {
        SDL_Log("Failed to make OpenGL context current: %s", SDL_GetError());
        return false;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_Log("Unable to initialize SDL_image: %s", IMG_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0) {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

	// Make sure we can create/compile shaders
	if (!LoadShaders()) {
		SDL_Log("Failed to load shaders.");
		return false;
	}

    // Create quad for drawing sprites
    CreateSpriteVerts();

    // Set the clear color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Create orthografic projection matrix
    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    
    // Set up base shader
    if (mBaseShader) {
        mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
        mBaseShader->SetActive();
    }

    // Set up sprite shader - needs to be set each frame in Draw()
    // ViewProj is set in Draw() to ensure it's current
	
	// Armazena dimensões da tela para o grid
	mScreenWidth = width;
	mScreenHeight = height;
	
	// Carrega o shader AdvancedGrid
	mAdvancedGridShader = new Shader();
	if (!mAdvancedGridShader->Load("../Shaders/AdvancedGrid")) {
		SDL_Log("Aviso: Falha ao carregar shader AdvancedGrid. Grid neon não estará disponível.");
		// Não retorna false, pois o jogo pode continuar sem o grid
	} else {
		// Cria o quad full-screen para o grid
		float vertices[] = {
			-1.0f, -1.0f,  // Canto inferior esquerdo
			 1.0f, -1.0f,  // Canto inferior direito
			 1.0f,  1.0f,  // Canto superior direito
			-1.0f,  1.0f   // Canto superior esquerdo
		};
		
		unsigned int indices[] = {
			0, 1, 2,  // Primeiro triângulo
			0, 2, 3   // Segundo triângulo
		};
		
		mFullScreenQuad = new VertexArray(vertices, 8, indices, 6);
		SDL_Log("Shader AdvancedGrid carregado com sucesso.");
	}

    return true;
}

void Renderer::UpdateScreenSize(float width, float height)
{
    mScreenWidth = width;
    mScreenHeight = height;
    
    // Atualizar viewport
    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    
    // Atualizar projeção ortográfica
    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    
    // Atualizar base shader
    if (mBaseShader) {
        mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
    }
}

void Renderer::UnloadData()
{
    // Destroy textures
    for (auto i : mTextures)
    {
        i.second->Unload();
        delete i.second;
    }
    mTextures.clear();

    // Destroy fonts
    for (auto i : mFonts)
    {
        i.second->Unload();
        delete i.second;
    }
    mFonts.clear();
}

void Renderer::Shutdown()
{
    UnloadData();

    delete mSpriteVerts;
    mSpriteVerts = nullptr;
    
    if (mSpriteShader) {
        mSpriteShader->Unload();
        delete mSpriteShader;
        mSpriteShader = nullptr;
    }
    
    if (mBaseShader) {
        mBaseShader->Unload();
        delete mBaseShader;
        mBaseShader = nullptr;
    }
    
    // Limpa recursos do AdvancedGrid
    if (mAdvancedGridShader) {
        mAdvancedGridShader->Unload();
        delete mAdvancedGridShader;
        mAdvancedGridShader = nullptr;
    }
    
    if (mFullScreenQuad) {
        delete mFullScreenQuad;
        mFullScreenQuad = nullptr;
    }

    SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
}

void Renderer::Clear()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", 1.0f); // Full opacity for normal objects

    vertices->SetActive();
    // For 2D line drawing, we use GL_LINE_LOOP with the base shader
    glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::Draw()
{
    // Enable depth buffering/disable alpha blend
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // Draw game components using base shader would go here if needed

    // Disable depth buffering
    glDisable(GL_DEPTH_TEST);

    // Enable alpha blending on the color buffer
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    // Activate sprite shader/verts
    if (mSpriteShader && mSpriteVerts) {
        mSpriteShader->SetActive();
        
        // Set view projection matrix for sprite shader (center-based coordinates)
        Matrix4 viewProj = Matrix4::CreateSimpleViewProj(mScreenWidth, mScreenHeight);
        mSpriteShader->SetMatrixUniform("uViewProj", viewProj);
        
        mSpriteVerts->SetActive();

        // Draw UI components
        for (auto ui : mUIComps)
        {
            ui->Draw(mSpriteShader);
        }
    }
}

void Renderer::DrawFilled(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", 1.0f); // Full opacity for normal objects

    vertices->SetActive();
    glDrawElements(GL_TRIANGLE_FAN, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawFilledWithAlpha(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color, float alpha)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", alpha);

    // Habilita blending para transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vertices->SetActive();
    glDrawElements(GL_TRIANGLE_FAN, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
    
    glDisable(GL_BLEND);
}

void Renderer::DrawAdvancedGrid(float screenWidth, float screenHeight, float time)
{
	// Verifica se o shader e o quad foram carregados
	if (!mAdvancedGridShader || !mFullScreenQuad) {
		return; // Grid não disponível, retorna silenciosamente
	}
	
	// Ativa o shader AdvancedGrid
	mAdvancedGridShader->SetActive();
	
	// Configura os uniforms necessários
	// uResolution: Resolução da tela (width, height)
	// Por que existe: O shader precisa saber o tamanho da tela para calcular
	//                 coordenadas de fragmento corretamente (gl_FragCoord.xy / uResolution)
	mAdvancedGridShader->SetVector2Uniform("uResolution", Vector2(screenWidth, screenHeight));
	
	// uTime: Tempo em segundos para animações
	// Por que existe: Permite animações como pulsação, movimento, etc.
	//                 O shader usa isso para criar efeitos dinâmicos
	mAdvancedGridShader->SetFloatUniform("uTime", time);
	
	// uColor: Cor base do grid neon (RGB)
	// Por que existe: Permite controlar a cor do grid (azul, ciano, rosa, etc.)
	//                 Valores sugeridos:
	//                 - Azul Tron: (0.0, 0.7, 1.0)
	//                 - Ciano: (0.0, 1.0, 1.0)
	//                 - Rosa Cyberpunk: (1.0, 0.0, 1.0)
	//                 - Verde Matrix: (0.0, 1.0, 0.0)
	Vector3 neonColor(0.0f, 0.7f, 1.0f); // Azul ciano estilo Tron
	mAdvancedGridShader->SetVectorUniform("uColor", neonColor);
	
	// Habilita blending para efeito de glow suave
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Desenha o quad full-screen
	// O fragment shader calculará o grid isométrico neon para cada pixel
	mFullScreenQuad->SetActive();
	glDrawElements(GL_TRIANGLES, mFullScreenQuad->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
	
	// Desabilita blending
	glDisable(GL_BLEND);
}

void Renderer::Present()
{
	// Swap the buffers
	SDL_GL_SwapWindow(mWindow);
}

void Renderer::AddUIElement(UIElement *comp)
{
    mUIComps.emplace_back(comp);

    std::sort(mUIComps.begin(), mUIComps.end(),[](UIElement* a, UIElement* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Renderer::RemoveUIElement(UIElement *comp)
{
    auto iter = std::find(mUIComps.begin(), mUIComps.end(), comp);
    if (iter != mUIComps.end()) {
        mUIComps.erase(iter);
    }
}

Texture* Renderer::GetTexture(const std::string& fileName)
{
    // Make sure OpenGL context is current before loading textures
    SDL_GLContext currentContext = SDL_GL_GetCurrentContext();
    if (currentContext != mContext) {
        if (SDL_GL_MakeCurrent(mWindow, mContext) != 0) {
            SDL_Log("Failed to make OpenGL context current when loading texture: %s", SDL_GetError());
            return nullptr;
        }
    }
    
    Texture* tex = nullptr;
    auto iter = mTextures.find(fileName);
    if (iter != mTextures.end())
    {
        tex = iter->second;
    }
    else
    {
        tex = new Texture();
        if (tex->Load(fileName))
        {
            mTextures.emplace(fileName, tex);
            return tex;
        }
        else
        {
            delete tex;
            return nullptr;
        }
    }
    return tex;
}

Font* Renderer::GetFont(const std::string& fileName)
{
    auto iter = mFonts.find(fileName);
    if (iter != mFonts.end())
    {
        return iter->second;
    }
    else
    {
        Font* font = new Font();
        if (font->Load(fileName))
        {
            mFonts.emplace(fileName, font);
        }
        else
        {
            font->Unload();
            delete font;
            font = nullptr;
        }
        return font;
    }
}

void Renderer::CreateSpriteVerts()
{
    float vertices[] = {
            -0.5f, 0.5f,  0.f, 0.f, 0.f, 0.0f, 0.f, 0.f, // top left
            0.5f,  0.5f,  0.f, 0.f, 0.f, 0.0f, 1.f, 0.f, // top right
            0.5f,  -0.5f, 0.f, 0.f, 0.f, 0.0f, 1.f, 1.f, // bottom right
            -0.5f, -0.5f, 0.f, 0.f, 0.f, 0.0f, 0.f, 1.f	 // bottom left
    };

    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    mSpriteVerts = new VertexArray(vertices, 4, indices, 6);
}

bool Renderer::LoadShaders()
{
    // Create sprite shader for UI
    mSpriteShader = new Shader();
    if (!mSpriteShader->Load("../Shaders/Sprite"))
    {
        return false;
    }

    mSpriteShader->SetActive();

	// Create base shader for game
	mBaseShader = new Shader();
	if (!mBaseShader->Load("../Shaders/Base")) {
		return false;
	}

	mBaseShader->SetActive();

    return true;
}