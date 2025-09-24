#version 460 core

in vec2 vTexcoord;
out vec4 FragColor;

uniform sampler2D screenMap;
uniform int isHorizontal;
uniform float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
uniform float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

void main()
{
	vec2 tex_offset = 1.0 / textureSize(screenMap, 0); // gets size of single texel
    vec3 result = texture(screenMap, vTexcoord).rgb * weight[0]; // current fragment's contribution
    if(isHorizontal == 1.0)
    {       
        for(int i = 1; i < 3; ++i)
        {
            float texelXOffset = tex_offset.x * i * offset[i];
            result += texture(screenMap, vTexcoord + vec2(texelXOffset, 0.0)).rgb * weight[i];
            result += texture(screenMap, vTexcoord - vec2(texelXOffset, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 3; ++i)
        {
            float texelXOffset = tex_offset.y * i * offset[i];
            result += texture(screenMap, vTexcoord + vec2(0.0, texelXOffset)).rgb * weight[i];
            result += texture(screenMap, vTexcoord - vec2(0.0, texelXOffset)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}