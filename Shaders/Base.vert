// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE.txt for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// Vertex shader base para transformação de vértices 2D com projeção ortográfica

layout (location = 0) in vec2 inPosition;

uniform mat4 uWorldTransform;
uniform mat4 uOrthoProj;
uniform vec3 uColor;

void main()
{
	gl_Position = uOrthoProj * uWorldTransform * vec4(inPosition, 0.0, 1.0);
}