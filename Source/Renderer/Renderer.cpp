#include <GL/glew.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include <fstream>
#include <cstring>

Renderer::Renderer(SDL_Window *window)
: mBaseShader(nullptr)
, mAdvancedGridShader(nullptr)
, mFullScreenQuad(nullptr)
, mScreenWidth(0.0f)
, mScreenHeight(0.0f)
, mWindow(window)
, mContext(nullptr)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(float width, float height)
{
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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
	mBaseShader->SetActive();
	
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

    return true;
}

void Renderer::Shutdown()
{
    mBaseShader->Unload();
    delete mBaseShader;
    mBaseShader = nullptr;
    
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
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Draw(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", 1.0f);

    vertices->SetActive();
    glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
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

void Renderer::Present()
{
	SDL_GL_SwapWindow(mWindow);
}

bool Renderer::LoadShaders()
{
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