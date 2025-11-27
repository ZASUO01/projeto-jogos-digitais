//
// Created for floor with grid
//

#include "Floor.h"
#include "../Game.h"
#include "../Components/GridDrawComponent.h"

Floor::Floor(class Game* game)
    : Actor(game)
{
    // Create grid component that covers the entire screen
    new GridDrawComponent(this, 
                         static_cast<float>(Game::WINDOW_WIDTH), 
                         static_cast<float>(Game::WINDOW_HEIGHT), 
                         50.0f,  // cell size
                         0,      // draw order (lowest = drawn first/background)
                         Vector3(1.0f, 1.0f, 1.0f)); // white color
    
    // Position at origin
    SetPosition(Vector2::Zero);
}

Floor::~Floor()
{
}

