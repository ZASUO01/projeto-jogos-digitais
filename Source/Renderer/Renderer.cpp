#include <GL/glew.h>
#include <SDL.h>
#include "Renderer.h"
#include "PlatformCompatibility.h"
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
, mScreenWidth(1024.0f)
, mScreenHeight(768.0f)
, mSpriteVerts(nullptr)
, mWindow(window)
,     mContext(nullptr)
{
}

Renderer::~Renderer()
{
}

// Inicializa o renderer OpenGL, shaders e recursos de renderização
bool Renderer::Initialize(float width, float height)
{
    mScreenWidth = width;
    mScreenHeight = height;

    // Configurar atributos OpenGL com fallback para compatibilidade cross-platform
    // Tentar OpenGL 3.3 primeiro, depois 3.2 se falhar (necessário para macOS)
    int majorVersion = 3;
    int minorVersion = 3;
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetSwapInterval(1);

    mContext = SDL_GL_CreateContext(mWindow);
    if (!mContext) {
        // Tentar fallback para OpenGL 3.2 (necessário para macOS mais antigo)
        SDL_Log("Failed to create OpenGL 3.3 context, trying 3.2: %s", SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        mContext = SDL_GL_CreateContext(mWindow);
        if (!mContext) {
            SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
            return false;
        }
        minorVersion = 2;
    }

    if (!EnsureOpenGLContextCurrent(mWindow, mContext)) {
        SDL_Log("Failed to make OpenGL context current");
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW: %s", glewGetErrorString(glewError));
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_Log("Unable to initialize SDL_image: %s", IMG_GetError());
        return false;
    }

    if (TTF_Init() != 0) {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));
    SDL_Log("OpenGL Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("GLEW Version: %s", glewGetString(GLEW_VERSION));

    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

	if (!LoadShaders()) {
		SDL_Log("Failed to load shaders.");
		return false;
	}

    CreateSpriteVerts();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    
    if (mBaseShader) {
        mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
        mBaseShader->SetActive();
    }
	
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
	
	mAdvancedGridShader = new Shader();
	std::string advancedGridPath = FindShaderPath("AdvancedGrid");
	if (!mAdvancedGridShader->Load(advancedGridPath)) {
		SDL_Log("Aviso: Falha ao carregar shader AdvancedGrid. Grid neon não estará disponível.");
	} else {
		SDL_Log("Shader AdvancedGrid carregado com sucesso.");
	}
	
	mCRTShader = new Shader();
	std::string crtPath = FindShaderPath("CRT");
	if (!mCRTShader->Load(crtPath)) {
		SDL_Log("Aviso: Falha ao carregar shader CRT. Efeito de TV antiga não estará disponível.");
	} else {
		SDL_Log("Shader CRT carregado com sucesso.");
	}
	
	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	
	glGenTextures(1, &mSceneTexture);
	glBindTexture(GL_TEXTURE_2D, mSceneTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 
	             0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mSceneTexture, 0);
	
	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 
	                      static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		SDL_Log("Erro: Frame Buffer não está completo!");
		return false;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	SDL_Log("Sistema de render-to-texture inicializado com sucesso.");

    return true;
}

// Atualiza o tamanho da tela e recalcula a projeção ortográfica
void Renderer::UpdateScreenSize(float width, float height)
{
    mScreenWidth = width;
    mScreenHeight = height;
    
    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    
    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    
    if (mBaseShader) {
        mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
    }
}

// Descarrega todas as texturas e fontes carregadas
void Renderer::UnloadData()
{
    for (auto i : mTextures)
    {
        i.second->Unload();
        delete i.second;
    }
    mTextures.clear();

    for (auto i : mFonts)
    {
        i.second->Unload();
        delete i.second;
    }
    mFonts.clear();
}

// Limpa todos os recursos do renderer
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

// Limpa os buffers de cor e profundidade
void Renderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Desenha uma forma usando linhas (wireframe)
void Renderer::Draw(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", 1.0f);

    vertices->SetActive();
    glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
}

// Desenha todos os elementos UI usando o sprite shader
void Renderer::Draw()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    if (mSpriteShader && mSpriteVerts) {
        mSpriteShader->SetActive();
        
        Matrix4 viewProj = Matrix4::CreateSimpleViewProj(mScreenWidth, mScreenHeight);
        mSpriteShader->SetMatrixUniform("uViewProj", viewProj);
        
        mSpriteVerts->SetActive();

        for (auto ui : mUIComps)
        {
            ui->Draw(mSpriteShader);
        }
    }
}

// Desenha uma forma preenchida (sólida)
void Renderer::DrawFilled(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", 1.0f);

    vertices->SetActive();
    glDrawElements(GL_TRIANGLE_FAN, vertices->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
}

// Desenha uma forma preenchida com transparência
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

// Desenha uma forma em wireframe com transparência
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

// Desenha o grid avançado isométrico neon como fundo
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

// Inicia renderização para textura (FBO)
void Renderer::BeginRenderToTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glViewport(0, 0, static_cast<GLsizei>(mScreenWidth), static_cast<GLsizei>(mScreenHeight));
}

// Finaliza renderização para textura e aplica efeito CRT
void Renderer::EndRenderToTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, static_cast<GLsizei>(mScreenWidth), static_cast<GLsizei>(mScreenHeight));
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (mCRTShader && mFullScreenQuad) {
		mCRTShader->SetActive();
		
		mCRTShader->SetVector2Uniform("uResolution", Vector2(mScreenWidth, mScreenHeight));
		mCRTShader->SetFloatUniform("uTime", SDL_GetTicks() / 1000.0f);
		mCRTShader->SetTextureUniform("uSceneTexture", mSceneTexture, 0);
		
		mFullScreenQuad->SetActive();
		glDrawElements(GL_TRIANGLES, mFullScreenQuad->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
	} else {
		glBindTexture(GL_TEXTURE_2D, mSceneTexture);
		if (!mCRTShader) {
			SDL_Log("Aviso: Shader CRT não disponível, efeito não será aplicado.");
		}
	}
}

// Apresenta o frame renderizado na tela
void Renderer::Present()
{
	SDL_GL_SwapWindow(mWindow);
}

// Adiciona um elemento UI e ordena por ordem de desenho
void Renderer::AddUIElement(UIElement *comp)
{
    mUIComps.emplace_back(comp);

    std::sort(mUIComps.begin(), mUIComps.end(),[](UIElement* a, UIElement* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

// Remove um elemento UI da lista
void Renderer::RemoveUIElement(UIElement *comp)
{
    auto iter = std::find(mUIComps.begin(), mUIComps.end(), comp);
    if (iter != mUIComps.end()) {
        mUIComps.erase(iter);
    }
}

// Carrega e retorna uma textura (usa cache se já foi carregada)
Texture* Renderer::GetTexture(const std::string& fileName)
{
    // Garantir que o contexto OpenGL está atual antes de carregar texturas
    // Importante para macOS e Linux onde o contexto pode ser perdido
    if (!EnsureOpenGLContextCurrent(mWindow, mContext)) {
        SDL_Log("Failed to make OpenGL context current when loading texture");
        return nullptr;
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

// Carrega e retorna uma fonte (usa cache se já foi carregada)
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

// Cria os vértices do sprite usado para renderizar UI
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

// Carrega todos os shaders necessários
bool Renderer::LoadShaders()
{
    mSpriteShader = new Shader();
    if (!mSpriteShader->Load(FindShaderPath("Sprite")))
    {
        return false;
    }

    mSpriteShader->SetActive();

	mBaseShader = new Shader();
	std::string baseShaderPath = FindShaderPath("Base");
	if (!mBaseShader->Load(baseShaderPath)) {
		SDL_Log("Erro: Não foi possível carregar o shader Base em: %s", baseShaderPath.c_str());
		return false;
	}

	mBaseShader->SetActive();
    return true;
}

// Procura o caminho correto para um shader testando múltiplos locais possíveis
std::string Renderer::FindShaderPath(const std::string& shaderName)
{
	std::vector<std::string> possiblePaths = {
		"../Shaders/" + shaderName,
		"Shaders/" + shaderName,
		"./Shaders/" + shaderName,
		"../../Shaders/" + shaderName,
	};
	
	char* basePath = SDL_GetBasePath();
	if (basePath) {
		std::string base(basePath);
		SDL_free(basePath);
		
		// Normalizar separadores de caminho para compatibilidade cross-platform
		std::replace(base.begin(), base.end(), '\\', '/');
		
		if (!base.empty() && base.back() == '/') {
			base.pop_back();
		}
		possiblePaths.push_back(base + "/Shaders/" + shaderName);
		size_t lastSlash = base.find_last_of('/');
		if (lastSlash != std::string::npos) {
			std::string parent = base.substr(0, lastSlash);
			possiblePaths.push_back(parent + "/Shaders/" + shaderName);
		}
	}
	
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
	
	SDL_Log("Aviso: Shader não encontrado em nenhum dos caminhos testados para: %s", shaderName.c_str());
	SDL_Log("Caminhos testados:");
	for (const auto& path : possiblePaths) {
		SDL_Log("  - %s", path.c_str());
	}
	return possiblePaths[0];
}