#version 460 core
#define MAX_LIGHT_COUNT 16

out vec4 FragColor;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexcoord;

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

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform float     shiness;

void main()
{
	vec3  camaraPos  = cameraBuffer.position.xyz;
	vec3  v			 = normalize(camaraPos - vPos);
	vec3  n			 = normalize(vNormal);
	vec3  diff		 = texture(diffuseMap, vTexcoord).rgb;
	float spec		 = texture(specularMap, vTexcoord).r;
	int   lightCount = lightBuffer.lights[0].lightCount;
	for(int i = 0; i < lightCount; i++)
	{
		vec3 lightPos = lightBuffer.lights[i].position.xyz;
		vec3 l = lightPos - vPos;
		float distance = length(l);
		l = normalize(l);
		vec3 h = normalize(l + v);
		vec3 lightColor = lightBuffer.lights[i].color.rgb;
		float constant = lightBuffer.lights[i].attentionFactor.r;
		float linear = lightBuffer.lights[i].attentionFactor.g;
		float quadratic = lightBuffer.lights[i].attentionFactor.b;
		vec3 ambient = 0.1 * lightColor * diff;
		vec3 diffuse = max(dot(n, l), 0.0) * lightColor * diff;
		vec3 specular = pow(max(dot(n, h), 0.0), 32) * lightColor * spec;
		float attenuation = 1.0 / (constant + linear * distance +quadratic * (distance * distance));
		FragColor += vec4((ambient + diffuse + specular) * attenuation, 0.0);
	}
}