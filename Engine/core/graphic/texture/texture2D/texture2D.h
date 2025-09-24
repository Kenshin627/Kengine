#pragma once
#include "typedef.h"
#include "../texture.h"



/// <summary>
/// no binding texture
/// </summary>
class Texture2D:public Texture
{
public:
	Texture2D(const TextureSpecification& spec = TextureSpecification());
	void loadFromData(uint width, uint height, const void* data, uint chanel, TextureInternalFormat internalFormat, TextureDataFormat dataFormat, uint dataType);
	void loadFromFile(const char* path, bool flipY = true, bool SRGB = false);
	virtual ~Texture2D() = default;
	Texture2D(const Texture2D& obj) = delete;
	Texture2D(Texture2D&& obj) = delete;
	Texture2D& operator=(const Texture2D& obj) = delete;	
};