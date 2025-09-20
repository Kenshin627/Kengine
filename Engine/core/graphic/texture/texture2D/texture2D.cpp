#include <glad/glad.h>
#include <gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "texture2D.h"
#include "core.h"

Texture2D::Texture2D(const TextureSpecification& spec)
	:Texture(spec)
{
	glCreateTextures(GL_TEXTURE_2D, 1, &mRendererID);	
	setWarpModeS(mWrapS);
	setWarpModeT(mWrapT);
	setMinFilter(mMinFilter);
	setMagFilter(mMagFilter);
	if (spec.warpS == TextureWarpMode::CLAMP_TO_BORDER || spec.warpT == TextureWarpMode::CLAMP_TO_BORDER)
	{
		glTextureParameterfv(mRendererID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(spec.mBorderColor));
	}
}

void Texture2D::loadFromData(uint width, uint height, const void* data, uint chanel, TextureInternalFormat internalFormat, TextureDataFormat dataFormat, uint dataType)
{
	if (!data)
	{	
		KS_CORE_ERROR("[TEXTURE LOAD FAILED]: data is nullptr");
	}
	mWidth  = width;
	mHeight = height;
	mChannels = chanel;
	mInternalFormat = internalFormat;
	mDataFormat = dataFormat;
	GLCALL(glTextureStorage2D(mRendererID, mMipmapLevels, Texture::convertToGLInternalFormat(mInternalFormat), width, height));
	GLCALL(glTextureSubImage2D(mRendererID, 0, 0, 0, width, height, Texture::convertToGLDataFormat(mDataFormat), dataType, data));
	if (mMipmapLevels > 1)
	{
		if (mMinFilter == TextureFilter::NEAREST_MIPMAP_NEAREST ||
			mMinFilter == TextureFilter::NEAREST_MIPMAP_LINEAR  ||
			mMinFilter == TextureFilter::LINEAR_MIPMAP_LINEAR   ||
			mMinFilter == TextureFilter::LINEAR_MIPMAP_NEAREST)
		{
			GLCALL(glGenerateTextureMipmap(mRendererID));
		}
		else
		{
			KS_CORE_WARN("[TEXTURE MIPMAP WARNING]: Mipmap levels > 1, but min filter is not a mipmap filter!");
		}
	}
}

void Texture2D::loadFromFile(const char* path, bool flipY)
{
	if (!path)
	{
		KS_CORE_ERROR("[TEXTURE LOAD FAILED]: path is nullptr");
		return;
	}
	stbi_set_flip_vertically_on_load(flipY);
	int width, height, channels;
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
	if (!data)
	{
		KS_CORE_ERROR("[TEXTURE LOAD FAILED]: Could not load image at path: {0}", path);
		return;
	}
	TextureInternalFormat internalFormat;
	TextureDataFormat dataFormat;
	switch (channels)
	{
		case 1: 
			internalFormat = TextureInternalFormat::R8;
			dataFormat = TextureDataFormat::R;
			break;
		case 3:
			internalFormat = TextureInternalFormat::RGB8;
			dataFormat = TextureDataFormat::RGB;
			break;
		case 4:
			internalFormat = TextureInternalFormat::RGBA8;
			dataFormat = TextureDataFormat::RGBA;
			break;
		default:
			break;
	}
	//TODO:采取更严格的判断SRGB的方式
	if (std::string(path).find("albedo") != std::string::npos)
	{
		if (channels == 4)
		{
			internalFormat = TextureInternalFormat::SRGB8ALPHA8;
			dataFormat = TextureDataFormat::RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = TextureInternalFormat::SRGB8;
			dataFormat = TextureDataFormat::RGB;
		}
		
	}
	//TODO: CHECK DATATYPE
	loadFromData(width, height, data, channels, internalFormat, dataFormat, GL_UNSIGNED_BYTE);
}
