#include <GL/glew.h>
#include <SDL.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Font.h"
#include "../UI/UIElement.h"
#include <algorithm>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <fstream>
#include <cstring>

Renderer::Renderer(SDL_Window *window)
: mSpriteShader(nullptr)
, mBaseShader(nullptr)
, mAdvancedGridShader(nullptr)
, mFullScreenQuad(nullptr)
, mCRTShader(nullptr)
, mFBO(0)
, mSceneTexture(0)
, mRBO(0)
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
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetSwapInterval(1);

    mContext = SDL_GL_CreateContext(mWindow);
    if (!mContext) {
        SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
        return false;
    }

    // Tornar o contexto atual (necessário no Linux)
    if (SDL_GL_MakeCurrent(mWindow, mContext) != 0) {
        SDL_Log("Failed to make OpenGL context current: %s", SDL_GetError());
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW: %s", glewGetErrorString(glewError));
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
    // Verificar versão do OpenGL
    SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));
    SDL_Log("OpenGL Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("GLEW Version: %s", glewGetString(GLEW_VERSION));

    // Configurar viewport (importante no Linux)
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

	if (!LoadShaders()) {
		SDL_Log("Failed to load shaders.");
		return false;
	}

    // Create quad for drawing sprites
    CreateSpriteVerts();

    // Set the clear color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    
    // Set up base shader
    if (mBaseShader) {
        mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
        mBaseShader->SetActive();
    }

    // Set up sprite shader - needs to be set each frame in Draw()
    // ViewProj is set in Draw() to ensure it's current
	
	mScreenWidth = width;
	mScreenHeight = height;
	
	mAdvancedGridShader = new Shader();
	std::string advancedGridPath = FindShaderPath("AdvancedGrid");
	if (!mAdvancedGridShader->Load(advancedGridPath)) {
		SDL_Log("Aviso: Falha ao carregar shader AdvancedGrid. Grid neon não estará disponível.");
	} else {
		float vertices[] = {
			-1.0f, -1.0f,
			 1.0f, -1.0f,
			 1.0f,  1.0f,
			-1.0f,  1.0f
		};
		
		unsigned int indices[] = {
			0, 1, 2,
			0, 2, 3
		};
		
		mFullScreenQuad = new VertexArray(vertices, 8, indices, 6);
		SDL_Log("Shader AdvancedGrid carregado com sucesso.");
	}
	
	// Inicializar shader CRT
	mCRTShader = new Shader();
	std::string crtPath = FindShaderPath("CRT");
	if (!mCRTShader->Load(crtPath)) {
		SDL_Log("Aviso: Falha ao carregar shader CRT. Efeito de TV antiga não estará disponível.");
	} else {
		SDL_Log("Shader CRT carregado com sucesso.");
	}
	
	// Criar Frame Buffer Object para render-to-texture
	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	
	// Criar textura para armazenar a cena renderizada
	glGenTextures(1, &mSceneTexture);
	glBindTexture(GL_TEXTURE_2D, mSceneTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 
	             0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Anexar textura ao FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mSceneTexture, 0);
	
	// Criar Render Buffer Object para depth/stencil (opcional, mas recomendado)
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 
	                      static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	
	// Verificar se o FBO está completo
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		SDL_Log("Erro: Frame Buffer não está completo!");
		return false;
	}
	
	// Voltar para o framebuffer padrão
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	SDL_Log("Sistema de render-to-texture inicializado com sucesso.");

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
    
    if (mAdvancedGridShader) {
        mAdvancedGridShader->Unload();
        delete mAdvancedGridShader;
        mAdvancedGridShader = nullptr;
    }
    
    if (mCRTShader) {
        mCRTShader->Unload();
        delete mCRTShader;
        mCRTShader = nullptr;
    }
    
    if (mFullScreenQuad) {
        delete mFullScreenQuad;
        mFullScreenQuad = nullptr;
    }
    
    // Limpar recursos do FBO
    if (mSceneTexture != 0) {
        glDeleteTextures(1, &mSceneTexture);
        mSceneTexture = 0;
    }
    
    if (mRBO != 0) {
        glDeleteRenderbuffers(1, &mRBO);
        mRBO = 0;
    }
    
    if (mFBO != 0) {
        glDeleteFramebuffers(1, &mFBO);
        mFBO = 0;
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
	mBaseShader->SetFloatUniform("uAlpha", 1.0f);

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
	mBaseShader->SetFloatUniform("uAlpha", 1.0f);

    vertices->SetActive();
    glDrawElements(GL_TRIANGLE_FAN, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawFilledWithAlpha(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color, float alpha)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", alpha);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vertices->SetActive();
    glDrawElements(GL_TRIANGLE_FAN, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
    
    glDisable(GL_BLEND);
}

void Renderer::DrawWithAlpha(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color, float alpha)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", alpha);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vertices->SetActive();
    glDrawElements(GL_LINES, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
    
    glDisable(GL_BLEND);
}

void Renderer::DrawAdvancedGrid(float screenWidth, float screenHeight, float time)
{
	if (!mAdvancedGridShader || !mFullScreenQuad) {
		return;
	}
	
	mAdvancedGridShader->SetActive();
	mAdvancedGridShader->SetVector2Uniform("uResolution", Vector2(screenWidth, screenHeight));
	mAdvancedGridShader->SetFloatUniform("uTime", time);
	
	Vector3 neonColor(0.0f, 0.7f, 1.0f);
	mAdvancedGridShader->SetVectorUniform("uColor", neonColor);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	mFullScreenQuad->SetActive();
	glDrawElements(GL_TRIANGLES, mFullScreenQuad->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
	
	glDisable(GL_BLEND);
}

void Renderer::BeginRenderToTexture()
{
	// Bind do FBO para renderizar a cena na textura
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glViewport(0, 0, static_cast<GLsizei>(mScreenWidth), static_cast<GLsizei>(mScreenHeight));
}

void Renderer::EndRenderToTexture()
{
	// Voltar para o framebuffer padrão (tela)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, static_cast<GLsizei>(mScreenWidth), static_cast<GLsizei>(mScreenHeight));
	
	// Limpar a tela antes de aplicar o efeito CRT
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Aplicar efeito CRT se o shader estiver disponível
	if (mCRTShader && mFullScreenQuad) {
		mCRTShader->SetActive();
		
		// Configurar uniforms do shader CRT
		mCRTShader->SetVector2Uniform("uResolution", Vector2(mScreenWidth, mScreenHeight));
		mCRTShader->SetFloatUniform("uTime", SDL_GetTicks() / 1000.0f);
		mCRTShader->SetTextureUniform("uSceneTexture", mSceneTexture, 0);
		
		// Desenhar o quad full-screen com o efeito CRT
		mFullScreenQuad->SetActive();
		glDrawElements(GL_TRIANGLES, mFullScreenQuad->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
	} else {
		// Se o shader CRT não estiver disponível, apenas copiar a textura diretamente
		// (fallback simples)
		glBindTexture(GL_TEXTURE_2D, mSceneTexture);
		// Nota: Para um fallback completo, seria necessário um shader simples de cópia
		// Por enquanto, apenas logamos um aviso
		if (!mCRTShader) {
			SDL_Log("Aviso: Shader CRT não disponível, efeito não será aplicado.");
		}
	}
}

void Renderer::Present()
{
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
	std::string baseShaderPath = FindShaderPath("Base");
	if (!mBaseShader->Load(baseShaderPath)) {
		SDL_Log("Erro: Não foi possível carregar o shader Base em: %s", baseShaderPath.c_str());
		return false;
	}

	mBaseShader->SetActive();
    return true;
}

std::string Renderer::FindShaderPath(const std::string& shaderName)
{
	// Lista de possíveis caminhos para os shaders
	std::vector<std::string> possiblePaths = {
		"../Shaders/" + shaderName,  // Caminho relativo do executável no build
		"Shaders/" + shaderName,     // Caminho relativo do diretório de trabalho
		"./Shaders/" + shaderName,   // Caminho explícito relativo
		"../../Shaders/" + shaderName, // Se executável estiver em subdiretório
	};
	
	// Tentar encontrar o executável e construir caminho absoluto
	char* basePath = SDL_GetBasePath();
	if (basePath) {
		std::string base(basePath);
		// Remover barra final se existir
		if (!base.empty() && (base.back() == '/' || base.back() == '\\')) {
			base.pop_back();
		}
		possiblePaths.push_back(base + "/Shaders/" + shaderName);
		// Tentar diretório pai
		size_t lastSlash = base.find_last_of("/\\");
		if (lastSlash != std::string::npos) {
			std::string parent = base.substr(0, lastSlash);
			possiblePaths.push_back(parent + "/Shaders/" + shaderName);
		}
		SDL_free(basePath);
	}
	
	// Verificar cada caminho usando ifstream (mais compatível)
	for (const auto& path : possiblePaths) {
		std::string vertPath = path + ".vert";
		std::string fragPath = path + ".frag";
		
		std::ifstream vertFile(vertPath);
		std::ifstream fragFile(fragPath);
		
		if (vertFile.good() && fragFile.good()) {
			vertFile.close();
			fragFile.close();
			SDL_Log("Shader encontrado: %s", path.c_str());
			return path;
		}
		
		if (vertFile.is_open()) vertFile.close();
		if (fragFile.is_open()) fragFile.close();
	}
	
	// Se não encontrou, retornar o primeiro caminho (para mostrar erro apropriado)
	SDL_Log("Aviso: Shader não encontrado em nenhum dos caminhos testados para: %s", shaderName.c_str());
	SDL_Log("Caminhos testados:");
	for (const auto& path : possiblePaths) {
		SDL_Log("  - %s", path.c_str());
	}
	return possiblePaths[0];
}