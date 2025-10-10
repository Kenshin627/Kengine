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

	float gradientX = 0.0f;
	float gradientY = 0.0f;
	vec2 texelSize = 1.0 / textureSize(screenMap, 0);
	for(int x = -1; x <=1; x++)
	{
		for(int y = -1; y <=1; y++)
		{
			vec2 offset = vec2(x, y) * texelSize;
			float gray = texture(screenMap, vTexcoord + offset).r;
			gradientX += gray * sobelX[x + 1][y + 1];
			gradientY += gray * sobelY[x + 1][y + 1];
		}
	}
	float gradientMagnitude = sqrt(gradientX * gradientX + gradientY * gradientY);
	float gradientDirection = atan(gradientY, gradientX); //[-pi, pi]
	FragColor = vec4(gradientMagnitude, gradientDirection, 0.0, 1.0);
}