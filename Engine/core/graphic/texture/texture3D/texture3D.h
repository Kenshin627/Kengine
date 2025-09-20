#pragma once
#include "../texture.h"

class Texture3D :public Texture
{
public:
	Texture3D(const TextureSpecification& spec);
	~Texture3D() = default;
private:
	uint mDepth;
};