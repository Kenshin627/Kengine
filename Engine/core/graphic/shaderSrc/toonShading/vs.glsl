#version 460 core

#define MAX_BONE_COUNT 1000
#define MAX_BONE_INFLUENCE 4

layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexcoord;
layout (location = 3) in vec3  aTangent;
layout (location = 4) in ivec4 aBoneIds;
layout (location = 5) in vec4  aWeights;

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

layout (std140, binding = 4) uniform AnimationBuffer
{
	mat4 boneMatrices[MAX_BONE_COUNT];
} animationBuffer;

uniform bool hasNormalTex;
uniform bool hasHeightTex;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexcoord;
out mat3 vTBN;

uniform bool      enablePOM;

//only has value in heightMap
out vec3 tangentSpaceViewPos;
out vec3 tangentSpaceFragPos;

void main()
{	
	if(hasHeightTex)
	{
		vec3 pos = vec3(modelMatrix * vec4(aPos, 1.0));
		vec3 n = normalize(mat3(modelMatrix) * aNormal);
		vec3 t = mat3(modelMatrix) * aTangent;
		t = normalize(t - n * dot(t, n));
		vec3 b = cross(n, t);
		mat3 tbn = transpose(mat3(t, b, n));
		if(enablePOM)
		{
			tangentSpaceViewPos = tbn * cameraBuffer.position.xyz;
			tangentSpaceFragPos = tbn * pos;
		}
	}

	if(hasNormalTex)
	{
		vec3 t = mat3(modelMatrix) * aTangent;
		vec3 n = normalize(mat3(modelMatrix) * aNormal);
		t = normalize(t - n * dot(t, n));
		vec3 b = cross(n, t);
		vTBN = mat3(t, b, n);
	}
	else
	{
		
		vNormal = normalize((vec3(modelMatrixInvertTranspose * vec4(aNormal, 0.0))));
	}
	vec4 animatedPos = vec4(0.0);
	bool hasBone = false;
	for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		if(aBoneIds[i] < 0)
		{
			continue;
		}
		if(aBoneIds[i] >= MAX_BONE_COUNT)
		{
			animatedPos = vec4(aPos, 1.0);
			break;
		}
		mat4 boneMat = animationBuffer.boneMatrices[aBoneIds[i]];
		vec4 localPos = boneMat * vec4(aPos, 1.0);
		animatedPos += localPos * aWeights[i];
		hasBone = true;
	}
	
	if(!hasBone)
	{
		animatedPos = vec4(aPos, 1.0);
	}
	vec4 worldPos = modelMatrix * animatedPos;
	vPos = worldPos.xyz / worldPos.w;
	vTexcoord = aTexcoord;
	gl_Position =  cameraBuffer.projectionMatrix * cameraBuffer.viewMatrix * worldPos;
}