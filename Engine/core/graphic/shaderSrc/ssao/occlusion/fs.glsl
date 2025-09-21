#version 460 core
#define MAX_SAMPLER_COUNT 128

const float bias = 0.6f;

out vec4 FragColor;

in vec2 vTexcoord;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

uniform float	  samperRadius;
uniform uint	  kernelSize;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D noise; //4x4
uniform vec3	  samplers[MAX_SAMPLER_COUNT];
uniform vec2	  frameBufferSize;

float rangeCheck(float samplerDepth, float targetDepth)
{
	return smoothstep(0.0, 1.0, samperRadius / abs(samplerDepth - targetDepth));
}

void main()
{
	vec2 texCoordScale = frameBufferSize / 4.0f;
	vec3 p		       = texture(gPosition, vTexcoord).xyz;
	vec3 n			   = normalize(texture(gNormal, vTexcoord).xyz);
	vec3 randomVec	   = texture(noise, vTexcoord * texCoordScale).xyz;
	vec3 t       	   = normalize(randomVec - n * dot(randomVec, n));
	vec3 b             = cross(n, t);
	mat3 tbn		   = mat3(t, b, n);
	float occlustion = 0.0f;
	for(int i = 0; i < kernelSize; i++)
	{	
		vec3 sampler   = tbn * samplers[i];
		//viewSpace
		sampler        = p + sampler * samperRadius;
		vec4 screenPos = cameraBuffer.projectionMatrix * vec4(sampler, 1.0);
		//NDC
		screenPos.xyz  = screenPos.xyz / screenPos.w;
		//[0,  1]
		screenPos.xyz  = screenPos.xyz * 0.5 + 0.5; 
		//depth in texture, nearest depth in face
		float samplerDepth = texture(gPosition, screenPos.xy).z;
		if(samplerDepth >= (sampler.z + bias))
		{
			occlustion += rangeCheck(samplerDepth, p.z);
		}
	}
	occlustion = 1.0 - (occlustion / kernelSize);
	FragColor = vec4(vec3(occlustion), 1.0);
}