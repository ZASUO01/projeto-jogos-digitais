#include "UITriangle.h"
#include "../Renderer/Shader.h"
#include "../Renderer/VertexArray.h"
#include "../Game.h"
#include "../Math.h"
#include <GL/glew.h>

UITriangle::UITriangle(class Game* game, const Vector2& offset, float size, float angle, int drawOrder)
    : UIElement(game, offset, 1.0f, angle, drawOrder)
    , mSize(size)
    , mColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f))
    , mTriangleVerts(nullptr)
{
    // Criar vertices para um triângulo equilátero apontando para cima
    // Formato: 3 vértices, mas precisamos usar formato sprite (4 vértices, 6 índices)
    // Vértices em coordenadas normalizadas (-0.5 a 0.5)
    // Formato: pos(3) + normal(3) + texCoord(2) = 8 floats por vértice
    float vertices[] = {
        // Vértice 0: top (ponta)
        0.0f, 0.5f, 0.0f,    // posição (x, y, z)
        0.0f, 0.0f, 1.0f,    // normal (x, y, z)
        0.5f, 0.0f,          // texCoord (u, v)
        // Vértice 1: bottom left
        -0.5f, -0.5f, 0.0f,  // posição
        0.0f, 0.0f, 1.0f,    // normal
        0.0f, 1.0f,          // texCoord
        // Vértice 2: bottom right
        0.5f, -0.5f, 0.0f,   // posição
        0.0f, 0.0f, 1.0f,    // normal
        1.0f, 1.0f,          // texCoord
        // Vértice 3: duplicado do vértice 0 para formar quad (mas não usado)
        0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.5f, 0.0f
    };
    
    // Índices para formar triângulo (0, 1, 2)
    unsigned int indices[] = {0, 1, 2, 0, 2, 3};
    mTriangleVerts = new VertexArray(vertices, 4, indices, 6);
}

UITriangle::~UITriangle()
{
    if (mTriangleVerts)
    {
        delete mTriangleVerts;
        mTriangleVerts = nullptr;
    }
}

void UITriangle::Draw(class Shader* shader)
{
    if (!GetIsVisible() || !mTriangleVerts)
        return;
    
    // Scale the triangle
    Matrix4 scaleMat = Matrix4::CreateScale(mSize * mScale, mSize * mScale, 1.0f);
    
    Matrix4 rotMat = Matrix4::CreateRotationZ(mAngle);
    
    // Translate to position on screen
    Matrix4 transMat = Matrix4::CreateTranslation(Vector3(mOffset.x, mOffset.y, 0.0f));
    
    // Set world transform
    Matrix4 world = scaleMat * rotMat * transMat;
    shader->SetMatrixUniform("uWorldTransform", world);
    
    // Set uTextureFactor and color
    shader->SetFloatUniform("uTextureFactor", 0.0f);
    shader->SetVectorUniform("uBaseColor", mColor);
    
    // Set texture uniform (even though we're not using a texture, the shader expects it)
    shader->SetTextureUniform("uTexture", 0);
    
    // Draw triangle (usar apenas os primeiros 3 índices para formar um triângulo)
    mTriangleVerts->SetActive();
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
}

