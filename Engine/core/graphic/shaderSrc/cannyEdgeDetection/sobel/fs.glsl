#version 460 core

//RG16F
//R: gradient magnitude
//G: gradient direction
out vec4 FragColor;

in vec2 vTexcoord;

uniform sampler2D screenMap;

void main()
{
	vec3 sobelX[3] = vec3[3]
	(
		vec3(-1.0, 0.0, 1.0),
		vec3(-2.0, 0.0, 2.0),
		vec3(-1.0, 0.0, 1.0)
	);

	vec3 sobelY[3] = vec3[3]
	(
		 vec3(-1.0, -2.0, -1.0),
		 vec3( 0.0,  0.0,  0.0),
		 vec3( 1.0,  2.0,  1.0)
	);

	vec3 gradientX = vec3(0.0f);
	vec3 gradientY = vec3(0.0f);
	vec2 texelSize = 1.0 / textureSize(screenMap, 0);
	for(int x = -1; x <=1; x++)
	{
		for(int y = -1; y <=1; y++)
		{
			vec2 offset = vec2(x, y) * texelSize;
			vec3 color = texture(screenMap, vTexcoord + offset).rgb;
			gradientX += color * sobelX[x + 1][y + 1];
			gradientY += color * sobelY[x + 1][y + 1];
		}
	}
	float xlen = length(gradientX);
	float ylen = length(gradientY);
	float gradientMagnitude = sqrt(xlen * xlen + ylen * ylen);
	float gradientDirection = atan(ylen, xlen); //[-pi, pi]
	FragColor = vec4(gradientMagnitude, gradientDirection, 0.0, 1.0);
}