#include "Texture.h"
#include <SDL_image.h>

Texture::Texture()
: mTextureID(0)
, mWidth(0)
, mHeight(0)
{
}

Texture::~Texture()
{
}

bool Texture::Load(const std::string &filePath)
{
    // Verify OpenGL context is current
    SDL_GLContext currentContext = SDL_GL_GetCurrentContext();
    if (!currentContext) {
        SDL_Log("Failed to load texture %s: No OpenGL context is current", filePath.c_str());
        return false;
    }

    SDL_Surface* surface = IMG_Load(filePath.c_str());

    if (!surface) {
        SDL_Log("Failed to load image %s: %s", filePath.c_str(), IMG_GetError());
        return false;
    }

    // Verify surface has valid pixels
    if (!surface->pixels) {
        SDL_Log("Failed to load texture %s: Surface has no pixel data", filePath.c_str());
        return false;
    }

    mWidth = surface->w;
    mHeight = surface->h;

    // Convert surface to RGBA format if necessary (for consistent format)
    SDL_Surface* convertedSurface = nullptr;
    if (surface->format->format != SDL_PIXELFORMAT_RGBA32 && 
        surface->format->format != SDL_PIXELFORMAT_BGRA32) {
        convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        if (!convertedSurface || !convertedSurface->pixels) {
            SDL_Log("Failed to convert surface format for texture %s", filePath.c_str());
            if (convertedSurface) SDL_FreeSurface(convertedSurface);
            return false;
        }
        surface = convertedSurface;
    }

    // Generate a GL texture
    glGenTextures(1, &mTextureID);
    if (mTextureID == 0) {
        SDL_Log("Failed to generate texture ID for %s", filePath.c_str());
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    
    // Check for OpenGL errors before glTexImage2D
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SDL_Log("OpenGL error before glTexImage2D for %s: %d", filePath.c_str(), error);
    }
    
    // Determine format based on pixel format
    GLenum glFormat = GL_RGBA;
    if (surface->format->format == SDL_PIXELFORMAT_BGRA32) {
        glFormat = GL_BGRA;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, glFormat, GL_UNSIGNED_BYTE, surface->pixels);
    
    // Check for OpenGL errors after glTexImage2D
    error = glGetError();
    if (error != GL_NO_ERROR) {
        SDL_Log("OpenGL error after glTexImage2D for %s: %d", filePath.c_str(), error);
        glDeleteTextures(1, &mTextureID);
        mTextureID = 0;
        return false;
    }

    // Generate mipmaps for texture
    glGenerateMipmap(GL_TEXTURE_2D);

    // Enable linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

void Texture::CreateFromSurface(SDL_Surface* surface)
{
    mWidth = surface->w;
    mHeight = surface->h;

    // Generate a GL texture
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 surface->pixels);

    // Use linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::Unload()
{
	glDeleteTextures(1, &mTextureID);
}

void Texture::SetActive(int index) const
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, mTextureID);
}

