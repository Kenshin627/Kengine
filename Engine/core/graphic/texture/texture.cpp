#include <glad/glad.h>
#include "texture.h"
#include "core.h"

Texture::Texture(const TextureSpecification& spec)
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
	
}


void Texture::bind(uint slot) const
{
	GLCALL(glBindTextureUnit(slot, mRendererID));
}

void Texture::unBind() const
{
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

int Texture::width() const
{
	return mWidth;
}

int Texture::height() const
{
	return mHeight;
}

uint Texture::id() const
{
	return mRendererID;
}

void Texture::setWarpModeS(TextureWarpMode mode)
{
	GLCALL(glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_S, convertToGLWarpMode(mode)));
}

void Texture::setWarpModeT(TextureWarpMode mode)
{
	GLCALL(glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_T, convertToGLWarpMode(mode)));
}

void Texture::setMinFilter(TextureFilter filter)
{
	GLCALL(glTextureParameteri(mRendererID, GL_TEXTURE_MIN_FILTER, convertToGLFilter(filter)));
}

void Texture::setMagFilter(TextureFilter filter)
{
	GLCALL(glTextureParameteri(mRendererID, GL_TEXTURE_MAG_FILTER, convertToGLFilter(filter)));
}

uint Texture::convertToGLInternalFormat(TextureInternalFormat format)
{
	switch (format)
	{
	case TextureInternalFormat::R8:					return   GL_R8;
	case TextureInternalFormat::RGB8:				return   GL_RGB8;
	case TextureInternalFormat::RGBA8:				return   GL_RGBA8;
	case TextureInternalFormat::RGB16F:				return   GL_RGB16F;
	case TextureInternalFormat::RGB32F:				return   GL_RGB32F;
	case TextureInternalFormat::SRGB8:				return	 GL_SRGB8;
	case TextureInternalFormat::SRGB8ALPHA8:		return	 GL_SRGB8_ALPHA8;
	case TextureInternalFormat::RGBA16F:			return   GL_RGBA16F;
	case TextureInternalFormat::RGBA32F:			return   GL_RGBA32F;
	case TextureInternalFormat::DEPTH24STENCIL8:	return   GL_DEPTH24_STENCIL8;
	case TextureInternalFormat::DEPTH32:			return   GL_DEPTH_COMPONENT32F;
	default: KS_CORE_ERROR("[TEXTURE INTERNAL FORMAT ERROR]: Unsupported texture internal format!"); return 0;
	}
}

uint Texture::convertToGLDataFormat(TextureDataFormat format)
{
	switch (format)
	{
	case TextureDataFormat::R:						return GL_RED;
	case TextureDataFormat::RGB:					return GL_RGB;
	case TextureDataFormat::RGBA:					return GL_RGBA;
	default: KS_CORE_ERROR("[TEXTURE DATA FORMAT ERROR]: Unsupported texture data format!"); return 0;
	}
}

uint Texture::convertToGLWarpMode(TextureWarpMode mode)
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

uint Texture::convertToGLFilter(TextureFilter filter)
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

