#version 460 core
#define MAX_LIGHT_COUNT 16

out vec4 FragColor;

uniform vec3 faceColor;
uniform vec3 lineColor;
uniform float thickness;
uniform float smoothing;

in vec3 gPos;
in vec3 gNormal;
in vec2 baryCentric;

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
	vec3 faceShadeColor = vec3(0.0);
	for(int i = 0; i < lightCount; i++)
	{
		vec3 lightPos = lightBuffer.lights[i].position.xyz;
		vec3 l = lightPos - gPos;
		float distance = length(l);
		l = normalize(l);
		vec3 n = normalize(gNormal);
		vec3 lightColor = lightBuffer.lights[i].color.rgb;
		float constant = lightBuffer.lights[i].attentionFactor.r;
		float linear = lightBuffer.lights[i].attentionFactor.g;
		float quadratic = lightBuffer.lights[i].attentionFactor.b;
		vec3 ambient = 0.1 * lightColor * faceColor;
		vec3 diffuse = max(dot(n, l), 0.0) * lightColor * faceColor;		
		float attenuation = 1.0 / (constant + linear * distance +quadratic * (distance * distance));
		faceShadeColor += (ambient + diffuse) * attenuation;
	}

	vec3 baries;
	baries.xy = baryCentric;
	baries.z = 1.0 - baryCentric.x - baryCentric.y;
	vec3 delta = fwidth(baries);
	vec3 thick = delta * thickness;
	vec3 smoothness = delta * smoothing;
	vec3 edgeFactor = smoothstep(thick - smoothness, thick + smoothness, baries);
	float wireframeFactor = min(min(edgeFactor.x, edgeFactor.y), edgeFactor.z);
	//wireframe + flat
	//FragColor = vec4(mix(lineColor, faceShadeColor, wireframeFactor), 1.0);

	//wireframe
	FragColor = vec4(lineColor * wireframeFactor, 1.0);
}