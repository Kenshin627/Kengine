#version 460 core

out vec4 FragColor;

in vec2 vTexcoord;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	vec3 position;
} cameraBuffer;

uniform uint samperRadius;
uniform uint kernelSize;
uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D noise; //4x4
uniform vec3 samplers[64];
uniform vec2 frameBufferSize;

float rangeCheck(float samplerDepth, float targetDepth)
{
	return smoothstep(0.0, 1.0, samperRadius / abs(samplerDepth - targetDepth));
}

void main()
{
	vec2 texCoordScale = frameBufferSize / vec2(4.0);
	vec4 positionDepth = texture(gPositionDepth, vTexcoord);
	vec3 pos		   = positionDepth.xyz;
	vec3 n			   = normalize(texture(gNormal, vTexcoord).xyz);
	vec3 randomVec	   = texture(noise, vTexcoord * texCoordScale).xyz;
	vec3 tangent	   = normalize(randomVec - n * dot(randomVec, n));
	vec3 bitangent = cross(n, tangent);
	mat3 tbn = mat3(tangent, bitangent, n);
	float occlustion = 0.0;
	for(int i = 0; i < kernelSize; i++)
	{	
		vec3 sampler = tbn * samplers[i];
		sampler = pos + sampler * samperRadius;
		vec4 projectedSampler = cameraBuffer.projectionMatrix * vec4(sampler, 1.0);
		projectedSampler.xyz = projectedSampler.xyz / projectedSampler.w;//[-1, 1]
		projectedSampler.xyz = projectedSampler.xyz * 0.5 + 0.5; //[0, 1]
		//depth in texture, nearest depth in face
		float samplerDepth = -texture(gPositionDepth, projectedSampler.xy).w;
		if(samplerDepth >= sampler.z)
		{
			occlustion += rangeCheck(sampler.z, pos.z);
		}
	}
	occlustion = 1.0 - (occlustion / kernelSize);
	FragColor = vec4(vec3(occlustion), 1.0);
}