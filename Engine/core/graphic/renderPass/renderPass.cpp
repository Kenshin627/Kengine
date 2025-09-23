#include "core.h"
#include "renderPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"
#include "geometry/geometry.h"

RenderPass::RenderPass(const RenderState& state)
	:mRenderState(state),
	 mFrameBuffer(nullptr),
	 mProgram(nullptr)
{
	mSize.x = state.viewport.z;
	mSize.y = state.viewport.w;
}

void RenderPass::beginPass()
{
	checkPassReady();
	if (mRenderState.target == RenderTarget::SCREEN)
	{
		//TO SCREEN
		GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
	else if (mRenderState.target == RenderTarget::FRAMEBUFFER)
	{
		if (mFrameBuffer)
		{
			mFrameBuffer->bind();
		}
	}
	
	if (mProgram)
	{
		mProgram->bind();
	}	
	updateRenderState();
}

void RenderPass::runPass(Scene* scene)
{
}

void RenderPass::endPass()
{
	checkPassReady();
	if (mFrameBuffer)
	{
		mFrameBuffer->unBind();
	}
	if (mProgram)
	{
		mProgram->unBind();
	}	
}

void RenderPass::resize(uint width, uint height)
{
	//resize viewport
	mRenderState.viewport.z = width;
	mRenderState.viewport.w = height;
	//resize fbo if nessesary
	if (mFrameBuffer)
	{
		mFrameBuffer->resize(width, height);
	}
}

void RenderPass::updateRenderState() const
{
	glViewport(mRenderState.viewport.x, mRenderState.viewport.y, mRenderState.viewport.z, mRenderState.viewport.w);
	glClear(mRenderState.clearBits);
	glClearColor(mRenderState.clearColor.r, mRenderState.clearColor.g, mRenderState.clearColor.b, mRenderState.clearColor.a);
	glClearDepth(mRenderState.clearDepth);
	if (mRenderState.depthTest)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(mRenderState.depthFunc);
		glDepthMask(mRenderState.depthMask);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
	if (mRenderState.blend)
	{
		glEnable(GL_BLEND);
		//TODO:BLEND FUNC
	}
	else
	{
		glDisable(GL_BLEND);
	}
	if (mRenderState.cullFace)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(mRenderState.frontFace);
		glCullFace(mRenderState.cullFaceMode);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
	if (mRenderState.stencilTest)
	{
		glEnable(GL_STENCIL_TEST);
	}
	else
	{
		glDisable(GL_STENCIL_TEST);
	}
	if (mRenderState.scissorTest)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(mRenderState.scissorBox.x, mRenderState.scissorBox.y, mRenderState.scissorBox.z, mRenderState.scissorBox.w);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}
	glPolygonMode(GL_FRONT_AND_BACK, mRenderState.polygonMode);
}

void RenderPass::setRenderState(const RenderState& state)
{
	mRenderState = state;
}

bool RenderPass::checkPassReady()
{
	if (mRenderState.target == RenderTarget::FRAMEBUFFER && !mFrameBuffer)
	{
		KS_CORE_ERROR("RenderPass::checkPassReady: FrameBuffer is null!");
		return false;
	}
	return true;
}

void RenderPass::setLastPassFBOs(const std::initializer_list<FrameBuffer*>& fbo)
{
	mlastPassFrameBuffer = fbo;
}

std::shared_ptr<FrameBuffer> RenderPass::getCurrentFrameBuffer() const
{
	return mFrameBuffer;
}
