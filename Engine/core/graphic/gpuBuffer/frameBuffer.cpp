#include <glad/glad.h>
#include "core.h"
#include "frameBuffer.h"

FrameBuffer::FrameBuffer()
	:mRendererID(0),
	 mWidth(0),
	 mHeight(0),
	 mMaxAttachmentCount(0)
{
	GLCALL(glCreateFramebuffers(1, &mRendererID));
	checkMaxColorAttachmentt();
}

FrameBuffer::FrameBuffer(uint width, uint height, const std::initializer_list<FrameBufferSpecification>& attachmentSpecs)
	:mRendererID(0),
	 mWidth(width),
	 mHeight(height),
	 mMaxAttachmentCount(0),
	 mSpecifications(attachmentSpecs)
{
	GLCALL(glCreateFramebuffers(1, &mRendererID));
	checkMaxColorAttachmentt();
	buildAttachments();
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &mRendererID);
}

void FrameBuffer::bind() const
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, mRendererID));
}

void FrameBuffer::unBind() const
{
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

uint FrameBuffer::id() const
{
	return mRendererID;
}

void FrameBuffer::buildAttachments()
{
	//clear
	mColorAttachments.clear();
	if (mDepthStencilAttachment)
	{
		mDepthStencilAttachment.reset();
	}
	
	//build
	for (auto& spec : mSpecifications)
	{
		buildAttachment(spec);
	}
	//config drawBuffers
	if (mDrawBuffers.empty())
	{
		GLCALL(glDrawBuffer(GL_NONE));
	}
	else
	{
		GLCALL(glNamedFramebufferDrawBuffers(mRendererID, mDrawBuffers.size(), mDrawBuffers.data()));
	}
	//checkComplete
	bool success = isComplete();
	KS_CORE_INFO("[FRAMEBUFFER]: Build {0}x{1} Framebuffer {2}", mWidth, mHeight, success ? "Success" : "Failed");
}

void FrameBuffer::buildAttachment(const FrameBufferSpecification& attachmentSpec)
{
	if (attachmentSpec.attachmentType == AttachmentType::Color)
	{
		if (mColorAttachments.size() >= mMaxAttachmentCount)
		{
			KS_CORE_WARN("[FRAMEBUFFER WARNING]: Exceed max color attachment count!");
			return;
		}
		TextureSpecification texSpec;
		texSpec.width  = mWidth;
		texSpec.height = mHeight;
		texSpec.warpS = attachmentSpec.warpS;
		texSpec.warpT = attachmentSpec.warpT;
		texSpec.minFilter = attachmentSpec.minFilter;
		texSpec.magFilter = attachmentSpec.magFilter;
		texSpec.internalFormat = attachmentSpec.internalFormat;
		texSpec.dataFormat = attachmentSpec.dataFormat;
		texSpec.mipmapLevel = 1;

		auto texture = std::make_unique<Texture2D>(texSpec);
		GLCALL(glTextureStorage2D(texture->id(), 1, texture->convertToGLInternalFormat(texSpec.internalFormat), texSpec.width, texSpec.height));
		GLCALL(glNamedFramebufferTexture(mRendererID, GL_COLOR_ATTACHMENT0 + mColorAttachments.size(), texture->id(), 0));
		mColorAttachments.push_back(std::move(texture));
		mDrawBuffers.push_back(GL_COLOR_ATTACHMENT0 + mColorAttachments.size() - 1);
	}
	else if (attachmentSpec.attachmentType == AttachmentType::DepthStencil)
	{
		if (mDepthStencilAttachment)
		{
			KS_CORE_WARN("[FRAMEBUFFER WARNING]: Only one depth stencil attachment allowed!");
			return;
		}
		TextureSpecification texSpec;
		texSpec.width = mWidth;
		texSpec.height = mHeight;
		texSpec.warpS = attachmentSpec.warpS;
		texSpec.warpT = attachmentSpec.warpT;
		texSpec.minFilter = attachmentSpec.minFilter;
		texSpec.magFilter = attachmentSpec.magFilter;
		texSpec.internalFormat = attachmentSpec.internalFormat;
		texSpec.dataFormat = attachmentSpec.dataFormat;
		texSpec.mipmapLevel = 1;
		mDepthStencilAttachment = std::make_unique<Texture2D>(texSpec);
		GLCALL(glTextureStorage2D(mDepthStencilAttachment->id(), 1, mDepthStencilAttachment->convertToGLInternalFormat(texSpec.internalFormat), texSpec.width, texSpec.height));
		GLCALL(glNamedFramebufferTexture(mRendererID, GL_DEPTH_STENCIL_ATTACHMENT, mDepthStencilAttachment->id(), 0));
	}
	//TODO:DEPTH ATTACHMENT
	else
	{
		KS_CORE_WARN("[FRAMEBUFFER WARNING]: Unknown attachment type!");
	}

}

void FrameBuffer::addAttachment(const FrameBufferSpecification& spec)
{
	mSpecifications.push_back(spec);
	buildAttachment(spec);
}

void FrameBuffer::addAttachments(const std::initializer_list<FrameBufferSpecification>& specs)
{
	for (const auto& spec : specs)
	{
		addAttachment(spec);
	}
}

bool FrameBuffer::isComplete() const
{
	GLenum status = glCheckNamedFramebufferStatus(mRendererID, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::string errorMsg = "[Framebuffer incomplete]: ";
		switch (status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			errorMsg += "Incomplete attachment (无效的附件配置)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			errorMsg += "Missing attachment (没有任何有效附件)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			errorMsg += "Incomplete draw buffer (绘制缓冲索引无效)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			errorMsg += "Incomplete read buffer (读取缓冲索引无效)";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			errorMsg += "Incomplete multisample (采样数不匹配)";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			errorMsg += "Unsupported format combination (格式组合不被支持)";
			break;
		default:
			errorMsg += "Unknown error (未知错误，状态码: " + std::to_string(status) + ")";
		}
		KS_CORE_ERROR("[FRAMEBUFFER ERROR]: {0}", errorMsg);
		return false;
	}
	return true;
}

void FrameBuffer::resize(uint width, uint height)
{
	if (mWidth == width && mHeight == height)
	{
		return;
	}
	mWidth = width;
	mHeight = height;
	buildAttachments();
}

void FrameBuffer::checkMaxColorAttachmentt()
{
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &mMaxAttachmentCount);
}
