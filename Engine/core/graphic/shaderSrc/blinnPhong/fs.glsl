#version 460 core
#define MAX_LIGHT_COUNT 16

out vec4 FragColor;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;

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
	int lightCount;
} lightBuffer;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform float     shiness;

void main()
{
	vec3 camaraPos = cameraBuffer.position;
	vec3 v = normalize(camaraPos - vPos);
	vec3 n = normalize(vNormal);
	vec3 diff = texture(diffuseMap, vTexcoord).rgb;
	float spec = texture(specularMap, vTexcoord).r;
	int lightCount = lightBuffer.lightCount;
	for(int i = 0; i < 1; i++)
	{
		vec3 lightPos = lightBuffer.pointLights[i].position.xyz;
		vec3 l = lightPos - vPos;
		float distance = length(l);
		l = normalize(l);
		vec3 h = normalize(l + v);
		vec3 lightColor = lightBuffer.pointLights[i].color.rgb;
		float constant = lightBuffer.pointLights[i].factor.r;
		float linear = lightBuffer.pointLights[i].factor.g;
		float quadratic = lightBuffer.pointLights[i].factor.b;
		vec3 ambient = 0.1 * lightColor * diff;
		vec3 diffuse = max(dot(n, l), 0.0) * lightColor * diff;
		vec3 specular = pow(max(dot(n, h), 0.0), shiness) * lightColor * spec;
		float attenuation = 1.0 / (constant + linear * distance +quadratic * (distance * distance));
		FragColor += vec4((ambient + diffuse + specular) * attenuation, 0.0);
	}
	//FragColor = vec4(diff, 1.0);
}