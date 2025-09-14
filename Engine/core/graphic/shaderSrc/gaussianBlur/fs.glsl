#version 460 core

in vec2 vTexcoord;

uniform sampler2D screenMap;
uniform int isHorizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

out vec4 FragColor;

void main()
{
	//vec2 texelSize = 1.0 / textureSize(screenMap, 1);
	//vec3 col = texture(screenMap, vTexcoord).rgb * weight[0];
	//for(int i = 1; i< 5; i++)
	//{
	//	col += texture(screenMap, vTexcoord + vec2(i) * texelSize * vec2(isHorizontal, 1 - isHorizontal)).rgb * weight[i];
	//	col += texture(screenMap, vTexcoord - vec2(i) * texelSize * vec2(isHorizontal, 1 - isHorizontal)).rgb * weight[i];
	//}
	//FragColor = vec4(col, 1.0);

	vec2 tex_offset = 1.0 / textureSize(screenMap, 0); // gets size of single texel
    vec3 result = texture(screenMap, vTexcoord).rgb * weight[0]; // current fragment's contribution
    if(isHorizontal == 1.0)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(screenMap, vTexcoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(screenMap, vTexcoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(screenMap, vTexcoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(screenMap, vTexcoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}