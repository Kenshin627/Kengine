#version 460 core
#define MAX_LIGHT_COUNT 16

//TO SCREEN
layout (location = 0) out vec4 fragColor;

in vec2 vTexcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuseSpec;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
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
	vec4 lightCount;
} lightBuffer;

void main()
{
	vec3 fragmentPos =  texture(gPosition, vTexcoord).xyz;
	vec3 n   =  normalize(texture(gNormal, vTexcoord).xyz);
	vec3 camaraPos = cameraBuffer.position;
	vec3 v = normalize(camaraPos - fragmentPos);
	vec4 diffuseSpec = texture(gDiffuseSpec, vTexcoord);
	vec3 diff = diffuseSpec.rgb;
	float spec = diffuseSpec.a;
	int lightCount = int(lightBuffer.lightCount.r);
	//TODO: update pointsLightCount
	for(int i = 0; i < 3; i++)
	{
		vec3 lightPos = lightBuffer.pointLights[i].position.xyz;
		vec3 l = lightPos - fragmentPos;
		float distance = length(l);
		l = normalize(l);
		vec3 h = normalize(l + v);
		vec3 lightColor = lightBuffer.pointLights[i].color.rgb;
		float constant = lightBuffer.pointLights[i].factor.r;
		float linear = lightBuffer.pointLights[i].factor.g;
		float quadratic = lightBuffer.pointLights[i].factor.b;
		vec3 ambient = 0.1 * lightColor * diff;
		vec3 diffuse = max(dot(n, l), 0.0) * lightColor * diff;
		//TODO: store shiness to gbuffer
		vec3 specular = pow(max(dot(n, h), 0.0), 128) * lightColor * spec;
		float attenuation = 1.0 / (constant + linear * distance +quadratic * (distance * distance));
		fragColor += vec4((ambient + diffuse + specular) * attenuation, 0.0);
	}
}