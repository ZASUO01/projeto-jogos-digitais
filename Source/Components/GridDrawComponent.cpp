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
    // Create grid vertices
    std::vector<float> floatVertices;
    std::vector<unsigned int> indices;
    
    unsigned int index = 0;
    
    // Vertical lines - always start at x=0 and end at x=width
    // Draw lines at each cell boundary
    for (float x = 0.0f; x <= width; x += cellSize) {
        // Clamp to exact width boundary
        if (x > width) x = width;
        
        // Start point (top) at y=0
        floatVertices.push_back(x);
        floatVertices.push_back(0.0f);
        indices.push_back(index++);
        
        // End point (bottom) at y=height
        floatVertices.push_back(x);
        floatVertices.push_back(height);
        indices.push_back(index++);
        
        // Break if we've reached the boundary
        if (x >= width) break;
    }
    
    // Horizontal lines - always start at y=0 and end at y=height
    // Draw lines at each cell boundary
    for (float y = 0.0f; y <= height; y += cellSize) {
        // Clamp to exact height boundary
        if (y > height) y = height;
        
        // Start point (left) at x=0
        floatVertices.push_back(0.0f);
        floatVertices.push_back(y);
        indices.push_back(index++);
        
        // End point (right) at x=width
        floatVertices.push_back(width);
        floatVertices.push_back(y);
        indices.push_back(index++);
        
        // Break if we've reached the boundary
        if (y >= height) break;
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

