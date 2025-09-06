#pragma once
#include <memory>
#include <vector>
#include "typedef.h"
#include "graphic/texture/texture2D/texture2D.h"

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
};

class FrameBuffer
{
public:
	FrameBuffer();
	FrameBuffer(uint width, uint height, const std::initializer_list<FrameBufferSpecification>& attachmentSpecs);
	~FrameBuffer();
	void bind() const;
	void unBind() const;
	uint id() const;
	void addAttachment(const FrameBufferSpecification& attachmentSpec);
	void addAttachments(const std::initializer_list<FrameBufferSpecification>& attachmentSpecs);
	bool isComplete() const;
	void resize(uint width, uint height);
	void checkMaxColorAttachmentt();
private:
	void buildAttachment(const FrameBufferSpecification& attachmentSpec);
	void buildAttachments();
private:
	uint								    mRendererID;
	uint								    mWidth;
	uint								    mHeight;
	int 									mMaxAttachmentCount;
	std::vector<std::unique_ptr<Texture2D>> mColorAttachments;
	std::vector<FrameBufferSpecification>	mSpecifications;
	std::unique_ptr<Texture2D>				mDepthStencilAttachment;
	std::vector<uint>						mDrawBuffers;
};