// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE.txt for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// Fragment shader base para renderização de objetos com suporte a alpha e efeitos de glow

out vec4 outColor;
uniform vec3 uColor;
uniform float uAlpha;

void main()
{
	float alpha = uAlpha;
	if (alpha <= 0.0) {
		alpha = 1.0;
	}
	
	bool isGlow = uColor.g > 0.5 && uColor.b > 0.5 && uColor.r < 0.3 && alpha < 0.8;
	bool isBlackLine = uColor.r < 0.1 && uColor.g < 0.1 && uColor.b < 0.1 && alpha >= 0.9;
	
	vec3 finalColor;
	if (isGlow) {
		finalColor = uColor * 2.0;
		finalColor = clamp(finalColor, 0.0, 1.0);
	} else if (isBlackLine) {
		finalColor = vec3(0.0, 0.0, 0.0);
	} else {
		finalColor = uColor * 1.2;
		finalColor = clamp(finalColor, 0.0, 1.0);
	}
	
	outColor = vec4(finalColor, alpha);
}