//
// Created by Lucas N. Ferreira on 08/12/23.
//

#include "HUD.h"
#include "../../Game.h"
#include "../../PathResolver.h"
#include <string>

HUD::HUD(class Game* game, const std::string& fontName)
    :UIScreen(game, fontName)
    ,mHealth1(nullptr)
    ,mHealth2(nullptr)
    ,mHealth3(nullptr)
    ,mScore(nullptr)
{
	// Position in top-left corner, moved more to the right for full visibility
	// Screen coordinates: center is (0, 0), top-left is negative X and positive Y
	// WINDOW_WIDTH = 1024, WINDOW_HEIGHT = 768
	// Moved right to -200 to ensure full visibility of health bar
	float topLeftX = -200.0f;
	float healthBarY = 320.0f;  // Health bar at top
	float scoreTextY = 260.0f;   // Score below health bar
	
	// Add health bar images in specified order (Orange, Red, Blue, ShieldBar)
	// ShieldBar is the background (drawOrder 0 = drawn first), progress segments drawn on top
	// Set transparency for health bar images (alpha = 0.6 for more transparency)
	UIImage* shieldBar = AddImage(PathResolver::ResolvePath("Assets/HUD/ShieldBar.png"), Vector2(topLeftX, healthBarY), 0.75f, 0.0f, 0);
	shieldBar->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.6f)); // Background drawn first
	mHealth1 = AddImage(PathResolver::ResolvePath("Assets/HUD/ShieldOrange.png"), Vector2(topLeftX, healthBarY), 0.75f, 0.0f, 1);
	mHealth1->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.6f));
	mHealth2 = AddImage(PathResolver::ResolvePath("Assets/HUD/ShieldRed.png"), Vector2(topLeftX, healthBarY), 0.75f, 0.0f, 2);
	mHealth2->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.6f));
	mHealth3 = AddImage(PathResolver::ResolvePath("Assets/HUD/ShieldBlue.png"), Vector2(topLeftX, healthBarY), 0.75f, 0.0f, 3);
	mHealth3->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.6f));
	
	// Add score text label and counter below health bar
	// Position them close together to look like one continuous text
	AddText("score: ", Vector2(topLeftX, scoreTextY), 1.0f, 0.0f, 40, 1024, 10);
	
	// Score counter positioned right next to "score: " text (reduced spacing for continuity)
	// Approximate width of "score: " text is around 80-100 pixels at scale 1.0
	mScore = AddText("0", Vector2(topLeftX + 95.0f, scoreTextY), 1.0f, 0.0f, 40, 1024, 11);
}

void HUD::SetHealth(int health)
{
	// Update health bar visibility based on health value
	// Health 3: show all 3 segments (Orange, Red, Blue)
	// Health 2: show 2 segments (Orange, Red)
	// Health 1: show 1 segment (Red only)
	// Health 0: show no segments
	
	// mHealth1 = ShieldOrange.png, mHealth2 = ShieldRed.png, mHealth3 = ShieldBlue.png
	if (mHealth1)
	{
		mHealth1->SetIsVisible(health >= 2); // Orange shows at health 2 and 3
	}
	if (mHealth2)
	{
		mHealth2->SetIsVisible(health >= 1); // Red shows at health 1, 2, and 3
	}
	if (mHealth3)
	{
		mHealth3->SetIsVisible(health >= 3); // Blue shows only at health 3
	}
}

void HUD::SetScore(int score)
{
	if (mScore)
	{
		mScore->SetText(std::to_string(score));
	}
}
