#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "texture2D.h"
#include "core.h"

Texture2D::Texture2D(const TextureSpecification& spec)
	:mRendererID(0),
	 mWidth(spec.width),
	 mHeight(spec.height),
	 mChannels(spec.chanel),
	 mMipmapLevels(spec.mipmapLevel),
	 mInternalFormat(spec.internalFormat),
	 mDataFormat(spec.dataFormat),
	 mWrapS(spec.warpS),
	 mWrapT(spec.warpT),
	 mMinFilter(spec.minFilter),
	 mMagFilter(spec.magFilter)
{
	glCreateTextures(GL_TEXTURE_2D, 1, &mRendererID);
	setWarpModeS(mWrapS);
	setWarpModeT(mWrapT);
	setMinFilter(mMinFilter);
	setMagFilter(mMagFilter);

}

void Texture2D::loadFromData(uint width, uint height, const void* data, uint chanel, TextureInternalFormat internalFormat, TextureDataFormat dataFormat)
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
	GLCALL(glTextureStorage2D(mRendererID, mMipmapLevels, convertToGLInternalFormat(mInternalFormat), width, height));
	glTextureSubImage2D(mRendererID, 0, 0, 0, width, height, convertToGLDataFormat(mDataFormat), GL_UNSIGNED_BYTE, data);
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
	loadFromData(width, height, data, channels, internalFormat, dataFormat);
}

void Texture2D::bind(uint slot) const
{
	GLCALL(glBindTextureUnit(slot, mRendererID));
}

void Texture2D::unBind() const
{
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

int Texture2D::width() const
{
	return mWidth;
}

int Texture2D::height() const
{
	return mHeight;
}

uint Texture2D::id() const
{
	return mRendererID;
}

void Texture2D::setWarpModeS(TextureWarpMode mode)
{
	GLCALL(glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_S, convertToGLWarpMode(mode)));
}

void Texture2D::setWarpModeT(TextureWarpMode mode)
{
	GLCALL(glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_T, convertToGLWarpMode(mode)));
}

void Texture2D::setMinFilter(TextureFilter filter)
{
	GLCALL(glTextureParameteri(mRendererID, GL_TEXTURE_MIN_FILTER, convertToGLFilter(filter)));
}

void Texture2D::setMagFilter(TextureFilter filter)
{
	GLCALL(glTextureParameteri(mRendererID, GL_TEXTURE_MAG_FILTER, convertToGLFilter(filter)));
}

uint Texture2D::convertToGLInternalFormat(TextureInternalFormat format)
{
	switch (format)
	{
		case TextureInternalFormat::R8:					return   GL_R8;
		case TextureInternalFormat::RGB8:				return   GL_RGB8;
		case TextureInternalFormat::RGBA8:				return   GL_RGBA8;
		case TextureInternalFormat::RGB16F:				return   GL_RGB16F;
		case TextureInternalFormat::RGB32F:				return   GL_RGB32F;
		case TextureInternalFormat::RGBA16F:			return   GL_RGBA16F;
		case TextureInternalFormat::RGBA32F:			return   GL_RGBA32F;
		case TextureInternalFormat::DEPTH24STENCIL8:	return   GL_DEPTH24_STENCIL8;
		case TextureInternalFormat::DEPTH32:			return   GL_DEPTH_COMPONENT32F;
		default: KS_CORE_ERROR("[TEXTURE INTERNAL FORMAT ERROR]: Unsupported texture internal format!"); return 0;	
	}
}

uint Texture2D::convertToGLDataFormat(TextureDataFormat format)
{
	switch (format)
	{
	case TextureDataFormat::R:							return GL_RED;
		case TextureDataFormat::RGB:					return GL_RGB;
		case TextureDataFormat::RGBA:					return GL_RGBA;
		default: KS_CORE_ERROR("[TEXTURE DATA FORMAT ERROR]: Unsupported texture data format!"); return 0;
	}
}

uint Texture2D::convertToGLWarpMode(TextureWarpMode mode)
{
	switch (mode)
	{
		case TextureWarpMode::REPEAT:					return GL_REPEAT;
		case TextureWarpMode::MIRRORED_REPEAT:			return GL_MIRRORED_REPEAT;
		case TextureWarpMode::CLAMP_TO_EDGE:			return GL_CLAMP_TO_EDGE;
		case TextureWarpMode::CLAMP_TO_BORDER:			return GL_CLAMP_TO_BORDER;
		case TextureWarpMode::MIRRORED_CLAMP_TO_EDGE:   return GL_MIRROR_CLAMP_TO_EDGE;
		default: KS_CORE_ERROR("[TEXTURE WRAP MODE ERROR]: Unsupported texture wrap mode!"); return 0;
	}
}

uint Texture2D::convertToGLFilter(TextureFilter filter)
{
	switch (filter)
	{
		case TextureFilter::LINEAR:						return GL_LINEAR;
		case TextureFilter::NEAREST:					return GL_NEAREST;
		case TextureFilter::NEAREST_MIPMAP_NEAREST:		return GL_NEAREST_MIPMAP_NEAREST;
		case TextureFilter::NEAREST_MIPMAP_LINEAR:		return GL_NEAREST_MIPMAP_LINEAR;
		case TextureFilter::LINEAR_MIPMAP_LINEAR:		return GL_LINEAR_MIPMAP_LINEAR;
		case TextureFilter::LINEAR_MIPMAP_NEAREST:		return GL_LINEAR_MIPMAP_NEAREST;
		default: KS_CORE_ERROR("[TEXTURE FILTER ERROR]: Unsupported texture filter!"); return 0;
	}
}
