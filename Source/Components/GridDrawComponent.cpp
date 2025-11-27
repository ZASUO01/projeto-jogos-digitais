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
        
        // Enable blending for neon glow effect
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // First pass: Draw the glow (wider lines with cyan color and reduced alpha)
        Vector3 glowColor(0.0f, 0.7f, 1.0f); // Cyan-blue glow color
        renderer->GetBaseShader()->SetVectorUniform("uColor", glowColor);
        renderer->GetBaseShader()->SetFloatUniform("uAlpha", 0.3f); // Reduced alpha for subtle glow
        
        // Set wider line width for glow (note: glLineWidth may be limited by implementation)
        // If glLineWidth doesn't work, the glow effect will still be visible due to blending
        glLineWidth(4.0f);
        
        mGridArray->SetActive();
        glDrawElements(GL_LINES, mGridArray->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
        
        // Second pass: Draw the black line on top (thinner, opaque)
        Vector3 lineColor(0.0f, 0.0f, 0.0f); // Black line
        renderer->GetBaseShader()->SetVectorUniform("uColor", lineColor);
        renderer->GetBaseShader()->SetFloatUniform("uAlpha", 1.0f); // Full opacity
        
        // Set thinner line width for the actual line
        glLineWidth(1.0f);
        
        glDrawElements(GL_LINES, mGridArray->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
        
        glDisable(GL_BLEND);
    }
}

