#version 330

layout (location = 0) in vec2 inPosition;

uniform mat4 uWorldTransform;
uniform mat4 uOrthoProj;
uniform float uTime;

void main()
{
    // Pequena animação nas linhas do grid
    vec2 animatedPosition = inPosition;

    gl_Position = uOrthoProj * uWorldTransform * vec4(animatedPosition, 0.0, 1.0);
}