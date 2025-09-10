#version 460 core
//glm::vec3 Position;
//glm::vec3 Normal;
//glm::vec2 Texcoord;
//glm::vec3 tangent;
//glm::vec3 bitangent;
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;

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
//out vec3 vNormal;
out vec2 vTexcoord;
out mat3 vTBN;

void main()
{	
	vec3 t = mat3(modelMatrix) * aTangent;
	vec3 b = mat3(modelMatrix) * aBiTangent;
	vec3 n = mat3(modelMatrix) * aNormal;
	vTBN = mat3(t, b, n);
	mat4 modelViewMatrix = cameraBuffer.viewMatrix* modelMatrix;
	mat4 modelViewMatrixInverseTranspose = transpose(inverse(modelViewMatrix));
	vec4 viewPos = modelViewMatrix * vec4(aPos, 1.0);
	vPos = viewPos.xyz / viewPos.w;
	//vNormal = normalize((vec3(modelViewMatrixInverseTranspose * vec4(aNormal, 0.0))));
	vTexcoord = aTexcoord;
	gl_Position =  cameraBuffer.projectionMatrix * viewPos;
}