//
// Created for floor with grid
//

#include "Floor.h"
#include "../Game.h"
#include "../Components/GridDrawComponent.h"
#include "../Math.h"

Floor::Floor(class Game* game)
    : Actor(game)
{
    // Increase grid size considerably (3x the window size)
    float gridWidth = static_cast<float>(Game::WINDOW_WIDTH) * 3.0f;
    float gridHeight = static_cast<float>(Game::WINDOW_HEIGHT) * 3.0f;
    // Increase granularity by reducing cell size
    float cellSize = 20.0f;
    
    // Create grid component that can exceed screen boundaries
    new GridDrawComponent(this, 
                         gridWidth, 
                         gridHeight, 
                         cellSize,  // cell size (smaller = more granular)
                         0,         // draw order (lowest = drawn first/background)
                         Vector3(1.0f, 1.0f, 1.0f)); // white color
    
    // Center the isometric grid on screen
    // After isometric transformation, the grid will be centered
    // We offset by half the transformed dimensions to center it
    const float isoAngle = Math::ToRadians(30.0f);
    const float cosIso = Math::Cos(isoAngle);
    const float sinIso = Math::Sin(isoAngle);
    
    // Calculate the bounding box of the transformed grid
    // Corners after transformation
    Vector2 corner1 = Vector2((0.0f - 0.0f) * cosIso, (0.0f + 0.0f) * sinIso);
    Vector2 corner2 = Vector2((gridWidth - 0.0f) * cosIso, (gridWidth + 0.0f) * sinIso);
    Vector2 corner3 = Vector2((0.0f - gridHeight) * cosIso, (0.0f + gridHeight) * sinIso);
    Vector2 corner4 = Vector2((gridWidth - gridHeight) * cosIso, (gridWidth + gridHeight) * sinIso);
    
    // Find min and max
    float minX = Math::Min(Math::Min(corner1.x, corner2.x), Math::Min(corner3.x, corner4.x));
    float maxX = Math::Max(Math::Max(corner1.x, corner2.x), Math::Max(corner3.x, corner4.x));
    float minY = Math::Min(Math::Min(corner1.y, corner2.y), Math::Min(corner3.y, corner4.y));
    float maxY = Math::Max(Math::Max(corner1.y, corner2.y), Math::Max(corner3.y, corner4.y));
    
    // Center offset
    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    
    // Position to center the grid on screen
    SetPosition(Vector2(Game::WINDOW_WIDTH * 0.5f - centerX, Game::WINDOW_HEIGHT * 0.5f - centerY));
}

Floor::~Floor()
{
}

