#version 460 core

#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_COUNT 1000

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexcoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in ivec4 aBoneIds;
layout (location = 5) in vec4  aWeights;

layout (std140, binding = 3) uniform LightMatriceBuffer
{
	mat4 lightViewProjectionMatrix;
	mat4 lightViewMatrix;
} lightMatriceBuffer;


layout (std140, binding = 4) uniform AnimationBuffer
{
	mat4 boneMatrices[MAX_BONE_COUNT];
} animationBuffer;

uniform mat4 modelMatrix;

void main()
{
	vec4 animatedPos = vec4(0.0);
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
	}
	gl_Position =  lightMatriceBuffer.lightViewProjectionMatrix * modelMatrix * animatedPos;
}