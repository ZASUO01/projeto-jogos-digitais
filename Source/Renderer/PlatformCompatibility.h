#pragma once

#include <SDL.h>

// Utilitários para compatibilidade cross-platform com OpenGL
// Especialmente para macOS e Linux

#ifdef __APPLE__
    // macOS requer OpenGL 3.2+ (não 3.3)
    #define MIN_OPENGL_MAJOR_VERSION 3
    #define MIN_OPENGL_MINOR_VERSION 2
    // macOS pode ter problemas com alguns atributos OpenGL
    #define USE_OPENGL_COMPATIBILITY_PROFILE false
#elif defined(__linux__)
    // Linux pode ter drivers variados, tentar 3.3 primeiro, fallback para 3.2
    #define MIN_OPENGL_MAJOR_VERSION 3
    #define MIN_OPENGL_MINOR_VERSION 2
    #define USE_OPENGL_COMPATIBILITY_PROFILE false
#else
    // Windows e outros
    #define MIN_OPENGL_MAJOR_VERSION 3
    #define MIN_OPENGL_MINOR_VERSION 3
    #define USE_OPENGL_COMPATIBILITY_PROFILE false
#endif

// Função para verificar se o contexto OpenGL está válido e atual
// Importante para macOS e Linux onde o contexto pode ser perdido
bool IsOpenGLContextValid();

// Função para garantir que o contexto OpenGL está atual
// Necessário antes de criar recursos OpenGL (texturas, shaders, etc)
bool EnsureOpenGLContextCurrent(SDL_Window* window, SDL_GLContext context);

