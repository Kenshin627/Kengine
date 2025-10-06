#version 460 core

#define MAX_KERNENL 21

in vec2 vTexcoord;
out vec4 FragColor;

uniform sampler2D screenMap;
uniform int isHorizontal;
uniform float weight[MAX_KERNENL] = float[]
(
    0.07979165762662888,
    0.07821168005466461,
    0.07365698367357254,
    0.06664759665727615,
    0.05794063210487366,
    0.04839608818292618,
    0.038838766515254974,
    0.029946694150567055,
    0.022185057401657104,
    0.01579066552221775,
    0.010798626579344273,
    0.007095202803611755,
    0.004479086026549339,
    0.002716702874749899,
    0.00158315384760499,
    0.0008864052360877395,
    0.0004768367507494986,
    0.0002464537392370403,
    0.000122385288705118,
    0.00005839172808919102,
    0.000026767118470161222
);

uniform float scale;
uniform float strength;

void main()
{
	vec2 texelSize = 1.0 / textureSize(screenMap, 0); // gets size of single texel
    vec3 result = texture(screenMap, vTexcoord).rgb * weight[0]; // current fragment's contribution
    vec2 offset;
    float k;
    float t;
    for(int i = 1; i < MAX_KERNENL; i += 2)
    {
        k = weight[i] + weight[i + 1];
        t = weight[i + 1] / k;
        vec2 direction = vec2(isHorizontal, 1 - isHorizontal);
        offset = texelSize * direction * (float(i) + t) * scale;
        result += texture(screenMap, vTexcoord + offset).rgb * k * strength;
        result += texture(screenMap, vTexcoord - offset).rgb * k * strength;
    }

    FragColor = vec4(result, 1.0);
}