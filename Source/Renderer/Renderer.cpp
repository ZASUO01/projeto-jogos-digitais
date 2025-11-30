#include <GL/glew.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"

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

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }

	// Make sure we can create/compile shaders
	if (!LoadShaders()) {
		SDL_Log("Failed to load shaders.");
		return false;
	}

    // Set the clear color to light grey
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Create orthografic projection matrix
    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
	mBaseShader->SetActive();
	
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

void Renderer::Shutdown()
{
    mBaseShader->Unload();
    delete mBaseShader;
    mBaseShader = nullptr;
    
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
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Draw(const Matrix4 &modelMatrix, VertexArray* vertices, Vector3 color)
{
	mBaseShader->SetActive();
	mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
	mBaseShader->SetVectorUniform("uColor", color);
	mBaseShader->SetFloatUniform("uAlpha", 1.0f); // Full opacity for normal objects

    vertices->SetActive();
    glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
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

bool Renderer::LoadShaders()
{
	// Create sprite shader
	mBaseShader = new Shader();
	if (!mBaseShader->Load("../Shaders/Base")) {
		return false;
	}

	mBaseShader->SetActive();

    return true;
}