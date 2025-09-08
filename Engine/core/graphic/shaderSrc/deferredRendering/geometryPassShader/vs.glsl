#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 position;
} cameraBuffer;

uniform mat4 modelMatrix;
uniform mat4 modelMatrixInvertTranspose;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexcoord;

void main()
{	
	mat4 modelViewMatrix = cameraBuffer.viewMatrix* modelMatrix;
	mat4 modelViewMatrixInverseTranspose = transpose(inverse(modelViewMatrix));
	vec4 viewPos = modelViewMatrix * vec4(aPos, 1.0);
	vPos = viewPos.xyz / viewPos.w;
	vNormal = normalize((vec3(modelViewMatrixInverseTranspose * vec4(aNormal, 0.0))));
	vTexcoord = aTexcoord;
	gl_Position =  cameraBuffer.projectionMatrix * viewPos;
}