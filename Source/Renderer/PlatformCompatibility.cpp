#include "PlatformCompatibility.h"
#include <SDL.h>
#include <GL/glew.h>

bool IsOpenGLContextValid()
{
    // Verificar se há um contexto OpenGL atual
    SDL_GLContext currentContext = SDL_GL_GetCurrentContext();
    if (!currentContext) {
        return false;
    }
    
    // Verificar se o contexto ainda é válido tentando uma operação OpenGL simples
    // No macOS e Linux, o contexto pode ser invalidado em certas situações
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        // Limpar erros anteriores
        while (glGetError() != GL_NO_ERROR) {}
    }
    
    // Tentar obter a versão do OpenGL (operação que requer contexto válido)
    const GLubyte* version = glGetString(GL_VERSION);
    if (!version) {
        return false;
    }
    
    return true;
}

bool EnsureOpenGLContextCurrent(SDL_Window* window, SDL_GLContext context)
{
    if (!window || !context) {
        return false;
    }
    
    // Verificar se o contexto já está atual
    SDL_GLContext currentContext = SDL_GL_GetCurrentContext();
    SDL_Window* currentWindow = SDL_GL_GetCurrentWindow();
    
    if (currentContext == context && currentWindow == window) {
        // Contexto já está atual, verificar se ainda é válido
        if (IsOpenGLContextValid()) {
            return true;
        }
    }
    
    // Tornar o contexto atual
    if (SDL_GL_MakeCurrent(window, context) != 0) {
        SDL_Log("Failed to make OpenGL context current: %s", SDL_GetError());
        return false;
    }
    
    // Verificar se o contexto é válido após torná-lo atual
    if (!IsOpenGLContextValid()) {
        SDL_Log("OpenGL context is not valid after making it current");
        return false;
    }
    
    return true;
}

