#version 460 core
#define MAX_LIGHT_COUNT 16

//TO SCREEN
layout (location = 0) out vec4 FragColor;

in vec2 vTexcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecShiness;
uniform sampler2D ssaoMap;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 position;
} cameraBuffer;

struct PointLight
{
	vec4 position;
	vec4 color;
	vec4 factor;
};

layout (std140, binding = 1) uniform LightBuffer
{
	PointLight pointLights[MAX_LIGHT_COUNT];
} lightBuffer;

void main()
{
	//calc illumulation in vewSpace
	vec3 fragmentPos =  texture(gPosition, vTexcoord).xyz;
	vec3 n   =  normalize(texture(gNormal, vTexcoord).xyz);
	vec3 camaraPos = cameraBuffer.position;
	vec3 v = normalize(- fragmentPos);
	float ambientOcclusion = texture(ssaoMap, vTexcoord).r;
	vec3 diff = texture(gDiffuse, vTexcoord).rgb;
	vec4 specShiness = texture(gSpecShiness, vTexcoord);
	float spec = specShiness.r;
	float shiness = specShiness.a;
	//int lightCount = int(lightBuffer.lightCount.r);
	//TODO: update pointsLightCount
	for(int i = 0; i < 3; i++)
	{
		vec4 viewLightPos = cameraBuffer.viewMatrix * vec4(lightBuffer.pointLights[i].position.xyz, 1.0);
		vec3 lightPos = viewLightPos.xyz / viewLightPos.w;
		vec3 l = lightPos - fragmentPos;
		float d = length(l);
		l = normalize(l);
		vec3 h = normalize(l + v);
		vec3 lightColor = lightBuffer.pointLights[i].color.rgb;
		float constant = lightBuffer.pointLights[i].factor.r;
		float linear = lightBuffer.pointLights[i].factor.g;
		float quadratic = lightBuffer.pointLights[i].factor.b;
		vec3 ambient = 0.1 * lightColor * diff;
		vec3 diffuse = max(dot(n, l), 0.0) * lightColor * diff;
		//TODO: store shiness to gbuffer
		vec3 specular = pow(max(dot(n, h), 0.0), 256) * lightColor * 1.0;
		float attenuation = 1.0 / (constant + linear * d +quadratic * (d * d));
		diffuse   *= attenuation;
		specular  *= attenuation;
		ambient   *= ambientOcclusion;
		FragColor += vec4(ambient + diffuse + specular, 1.0);
	}
	//FragColor = pow(FragColor, vec4(0.4545));
}