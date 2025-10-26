#version 460 core
#define MAX_LIGHT_COUNT 16

out vec4 FragColor;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;

uniform bool hasNormalTex;
uniform bool hasDiffTex;

uniform vec3 albedoColor;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;
in mat3 vTBN;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 position;
	vec4 clipRange;
} cameraBuffer;

struct Light
{
	vec4	position;
	vec4	direction;
	vec4	color;
	vec4	attentionFactor;
	float	outterCutoff;
	float	innerCutoff;
	int		type;
	int		lightCount;
};

layout (std140, binding = 1) uniform LightBuffer
{
	Light lights[MAX_LIGHT_COUNT];
} lightBuffer;


void main()
{
	int lightCount = lightBuffer.lights[0].lightCount;
	vec3 n;
	if(hasNormalTex)
	{
		n = texture(normalMap, vTexcoord).xyz;
		n = n * 2.0 - 1.0;
		n = vTBN * n;
		n = normalize(n);
	}
	else
	{
		n = normalize(vNormal);
	}
	//TODO: support real lighting
	vec3 albedo;
	if(hasDiffTex)
	{
		albedo = texture(albedoMap, vTexcoord).rgb;
	}
	else
	{
		albedo = albedoColor;
	}
	vec3 ambient = vec3(0.03);
	vec3 specularColor = vec3(1.0);
	vec3 rimColor = vec3(1.0);
	float shinness = 32;
	vec3 l = vec3(0.0, -0.5, -0.5);
	vec3 lightColor = vec3(1.0);
	l = normalize(l);
	float NDotL = max(0.0, dot(n, l));
	float lightIntensity =  smoothstep(0.0, 0.01, NDotL);
	vec3 v = normalize(cameraBuffer.position.xyz - vPos);
	vec3 h = normalize(v + l);
	float NDotH = max(0.0, dot(n, h));
	NDotH = pow(NDotH, shinness);
	float spec = smoothstep(0.005, 0.01, NDotH);
	float rimDot = 1.0 - dot(v, n);
	float rim = rimDot * pow(NDotL, 0.1);
	vec3 c = rim * rimColor + spec * specularColor + ambient * albedo + albedo * lightIntensity;
	FragColor = vec4(c, 1.0);
}