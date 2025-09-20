#pragma once
#include <glm.hpp>
#include "typedef.h"

enum class TextureInternalFormat
{
	R8,
	RGB8,
	RGBA8,
	RGB16F,
	RGB32F,
	RGBA16F,
	RGBA32F,
	DEPTH32,
	SRGB8,          //albedo Texture need to gamma 2.2 first
	SRGB8ALPHA8,     //albedo Texture need to gamma 2.2 first
	DEPTH24STENCIL8
};

enum class TextureDataFormat
{
	R,
	RGB,
	RGBA,
	DEPTH,
	DELTHSTENCIL
};

enum class TextureWarpMode
{
	REPEAT,
	MIRRORED_REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER,
	MIRRORED_CLAMP_TO_EDGE
};

enum class TextureFilter
{
	LINEAR,
	NEAREST,
	NEAREST_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR,
	LINEAR_MIPMAP_NEAREST,
};

struct TextureSpecification
{
	uint				   width		  = 0;
	uint				   height		  = 0;
	uint				   depth		  = 1;
	TextureInternalFormat  internalFormat = TextureInternalFormat::RGB8;
	TextureDataFormat      dataFormat     = TextureDataFormat::RGB;
	TextureWarpMode		   warpS		  = TextureWarpMode::REPEAT;
	TextureWarpMode		   warpT		  = TextureWarpMode::REPEAT;
	TextureFilter		   minFilter	  = TextureFilter::LINEAR;
	TextureFilter		   magFilter	  = TextureFilter::LINEAR;
	uint				   chanel		  = 4;
	uint				   mipmapLevel	  = 1;
	glm::vec4		       mBorderColor	  = {0.0, 0.0, 0.0, 0.0};
};

class Texture
{
public:
	Texture(const TextureSpecification& spec);
	virtual ~Texture() = default;
	void bind(uint slot = 0) const;
	void unBind() const;
	int width() const;
	int height() const;
	uint id() const;
	void setWarpModeS(TextureWarpMode mode);
	void setWarpModeT(TextureWarpMode mode);
	void setMinFilter(TextureFilter filter);
	void setMagFilter(TextureFilter filter);
public:
	static uint convertToGLInternalFormat(TextureInternalFormat format);
	static uint convertToGLDataFormat(TextureDataFormat format);
	static uint convertToGLWarpMode(TextureWarpMode mode);
	static uint convertToGLFilter(TextureFilter filter);
protected:
	uint				  mRendererID = 0;
	uint				  mWidth = 0;
	uint				  mHeight = 0;
	uint				  mChannels = 3;
	uint				  mMipmapLevels;
	TextureInternalFormat mInternalFormat = TextureInternalFormat::RGB8;
	TextureDataFormat     mDataFormat = TextureDataFormat::RGB;
	TextureWarpMode		  mWrapS;
	TextureWarpMode		  mWrapT;
	TextureFilter		  mMinFilter;
	TextureFilter		  mMagFilter;
};