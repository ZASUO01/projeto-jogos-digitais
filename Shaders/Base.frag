// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE.txt for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// This corresponds to the output color to the color buffer
out vec4 outColor;
uniform vec3 uColor;
uniform float uAlpha; // Alpha uniform for neon effect (defaults to 1.0 if not set)

void main()
{
	// Get alpha value, defaulting to 1.0 if uniform is not set
	// OpenGL will use 0.0 if uniform is not set, so we check for that
	float alpha = uAlpha;
	if (alpha <= 0.0) {
		alpha = 1.0; // Default to full opacity
	}
	
	// Check if this is a glow pass (cyan/blue color with reduced alpha)
	bool isGlow = uColor.g > 0.5 && uColor.b > 0.5 && uColor.r < 0.3 && alpha < 0.8;
	
	// Check if this is the black line (should be opaque)
	bool isBlackLine = uColor.r < 0.1 && uColor.g < 0.1 && uColor.b < 0.1 && alpha >= 0.9;
	
	vec3 finalColor;
	if (isGlow) {
		// Glow effect: bright cyan-blue with enhanced brightness
		finalColor = uColor * 2.0; // Make glow brighter
		finalColor = clamp(finalColor, 0.0, 1.0);
	} else if (isBlackLine) {
		// Black line: keep it black
		finalColor = vec3(0.0, 0.0, 0.0);
	} else {
		// Normal objects: slightly brighter for visibility
		finalColor = uColor * 1.2;
		finalColor = clamp(finalColor, 0.0, 1.0);
	}
	
	outColor = vec4(finalColor, alpha);
}