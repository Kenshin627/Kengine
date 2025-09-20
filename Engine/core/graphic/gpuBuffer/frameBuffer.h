#pragma once
#include <memory>
#include <vector>
#include <glm.hpp>
#include "typedef.h"
#include "graphic/texture/texture.h"

enum class AttachmentType
{
	Color,
	Depth,
	Stencil,
	DepthStencil
};

struct FrameBufferSpecification
{
	AttachmentType		   attachmentType;
	TextureInternalFormat  internalFormat;
	TextureDataFormat      dataFormat;
	TextureWarpMode		   warpS;
	TextureWarpMode		   warpT;
	TextureFilter		   minFilter;
	TextureFilter		   magFilter;
	glm::vec4              borderColor;
};

class FrameBuffer
{
public:
	FrameBuffer();
	FrameBuffer(const glm::vec3& size, const std::initializer_list<FrameBufferSpecification>& attachmentSpecs);
	~FrameBuffer();
	void bind() const;
	void unBind() const;
	uint id() const;
	void addAttachment(const FrameBufferSpecification& attachmentSpec);
	void addAttachments(const std::initializer_list<FrameBufferSpecification>& attachmentSpecs);
	bool isComplete() const;
	void resize(uint width, uint height);
	void checkMaxColorAttachment();
	Texture* getColorAttachment(uint index) const;
	Texture* getDepthStencilAttachment() const;
private:
	void buildAttachment(const FrameBufferSpecification& attachmentSpec);
	void buildAttachments();
private:
	uint								    mRendererID;
	uint								    mWidth;
	uint								    mHeight;
	uint								    mDepth;
	int 									mMaxAttachmentCount;
	std::vector<std::unique_ptr<Texture>> mColorAttachments;
	std::vector<FrameBufferSpecification>	mSpecifications;
	std::unique_ptr<Texture>				mDepthStencilAttachment;
	std::vector<uint>						mDrawBuffers;
};