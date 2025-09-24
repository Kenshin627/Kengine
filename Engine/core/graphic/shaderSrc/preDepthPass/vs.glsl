#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aTangent;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

uniform mat4 modelMatrix;
uniform mat4 modelMatrixInvertTranspose;

out vec2 vTexcoord;

void main()
{
	vTexcoord = aTexcoord;
	gl_Position = cameraBuffer.projectionMatrix * cameraBuffer.viewMatrix * modelMatrix * vec4(aPos, 1.0);
}