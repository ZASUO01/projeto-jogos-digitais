//
// Created for grid drawing
//

#include "GridDrawComponent.h"
#include "../Game.h"
#include "../Renderer/Shader.h"
#include <GL/glew.h>
#include <cmath>

namespace {
    static std::vector<Vector2> emptyVertices;
}

GridDrawComponent::GridDrawComponent(class Actor* owner, float width, float height, float cellSize, int drawOrder, Vector3 color)
    : DrawComponent(owner, emptyVertices, drawOrder, color)
    , mWidth(width)
    , mHeight(height)
    , mCellSize(cellSize)
    , mGridArray(nullptr)
{
    // Create grid vertices with isometric projection
    std::vector<float> floatVertices;
    std::vector<unsigned int> indices;
    
    // Isometric transformation constants
    // Typical isometric projection: angle of ~30 degrees
    const float isoAngle = Math::ToRadians(30.0f);
    const float cosIso = Math::Cos(isoAngle);  // ~0.866
    const float sinIso = Math::Sin(isoAngle);  // ~0.5
    
    // Function to convert cartesian coordinates to isometric
    auto ToIsometric = [cosIso, sinIso](float x, float y) -> Vector2 {
        float isoX = (x - y) * cosIso;
        float isoY = (x + y) * sinIso;
        return Vector2(isoX, isoY);
    };
    
    unsigned int index = 0;
    
    // Calculate number of cells
    int numCellsX = static_cast<int>(width / cellSize) + 1;
    int numCellsY = static_cast<int>(height / cellSize) + 1;
    
    // Vertical lines (diagonal left-right in isometric view)
    for (int i = 0; i <= numCellsX; ++i) {
        float x = i * cellSize;
        if (x > width) x = width;
        
        // Start point at y=0
        Vector2 start = ToIsometric(x, 0.0f);
        floatVertices.push_back(start.x);
        floatVertices.push_back(start.y);
        indices.push_back(index++);
        
        // End point at y=height
        Vector2 end = ToIsometric(x, height);
        floatVertices.push_back(end.x);
        floatVertices.push_back(end.y);
        indices.push_back(index++);
    }
    
    // Horizontal lines (diagonal right-left in isometric view)
    for (int i = 0; i <= numCellsY; ++i) {
        float y = i * cellSize;
        if (y > height) y = height;
        
        // Start point at x=0
        Vector2 start = ToIsometric(0.0f, y);
        floatVertices.push_back(start.x);
        floatVertices.push_back(start.y);
        indices.push_back(index++);
        
        // End point at x=width
        Vector2 end = ToIsometric(width, y);
        floatVertices.push_back(end.x);
        floatVertices.push_back(end.y);
        indices.push_back(index++);
    }
    
    mGridArray = new VertexArray(floatVertices.data(), static_cast<unsigned int>(floatVertices.size()), 
                                 indices.data(), static_cast<unsigned int>(indices.size()));
}

GridDrawComponent::~GridDrawComponent()
{
    delete mGridArray;
    mGridArray = nullptr;
}

void GridDrawComponent::Draw(Renderer* renderer)
{
    if (mOwner->GetState() == ActorState::Active) {
        renderer->GetBaseShader()->SetActive();
        renderer->GetBaseShader()->SetMatrixUniform("uWorldTransform", mOwner->GetModelMatrix());
        renderer->GetBaseShader()->SetVectorUniform("uColor", mColor);
        
        mGridArray->SetActive();
        glDrawElements(GL_LINES, mGridArray->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
    }
}

