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
uniform int lightCount;

layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 viewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 position;
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
	//calc illumulation in vewSpace
	vec3 fragmentPos	   =  texture(gPosition, vTexcoord).xyz;
	vec3 n				   =  normalize(texture(gNormal, vTexcoord).xyz);
	vec3 v				   = normalize(-fragmentPos);
	float ambientOcclusion = texture(ssaoMap, vTexcoord).r;
	vec3 diff			   = texture(gDiffuse, vTexcoord).rgb;
	vec4 specShiness	   = texture(gSpecShiness, vTexcoord);
	float spec			   = specShiness.r;
	float shiness		   = specShiness.a;
	shiness *= 100;
	int lightCount = lightBuffer.lights[0].lightCount;
	for(int i = 0; i < lightCount; i++)
	{
		vec4 viewLightPos = cameraBuffer.viewMatrix * vec4(lightBuffer.lights[i].position.xyz, 1.0);
		vec3 lightPos = vec3(viewLightPos);
		vec3 l = lightPos - fragmentPos;
		float d = length(l);
		l = normalize(l);
		vec3 h = normalize(l + v);
		vec3 lightColor = lightBuffer.lights[i].color.rgb;
		float constant = lightBuffer.lights[i].attentionFactor.r;
		float linear = lightBuffer.lights[i].attentionFactor.g;
		float quadratic = lightBuffer.lights[i].attentionFactor.b;
		vec3 ambient = 0.01 * lightColor * diff;
		vec3 diffuse = max(dot(n, l), 0.0) * lightColor * diff;
		vec3 specular = pow(max(dot(n, h), 0.0), shiness) * lightColor * spec;
		float attenuation = 1.0 / (constant + linear * d +quadratic * (d * d));

		//check if spotLight
		float intensity = 1.0;
		if(lightBuffer.lights[i].type == 1)
		{
			vec3  lightDirection = normalize(vec3(cameraBuffer.viewMatrix * -lightBuffer.lights[i].direction));
			float costheta = dot(l, lightDirection);
			float epsilon = lightBuffer.lights[i].innerCutoff - lightBuffer.lights[i].outterCutoff;
			intensity = clamp((costheta - lightBuffer.lights[i].outterCutoff), 0.0, 1.0) / epsilon;
		}
		diffuse   *= attenuation;
		diffuse   *= intensity;
		specular  *= attenuation;
		specular  *= intensity;
		ambient   *= ambientOcclusion;
		FragColor += vec4(specular + ambient + diffuse, 1.0);
	}
	FragColor = pow(FragColor, vec4(0.4545));
}