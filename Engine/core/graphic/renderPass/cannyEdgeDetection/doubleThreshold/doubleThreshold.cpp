#include "doubleThreshold.h"
#include <graphic/program/program.h>
#include <graphic/gpuBuffer/frameBuffer.h>
#include "core.h"
#include "geometry/screenQuad.h"
#include "scene/scene.h"

DoubleThreshold::DoubleThreshold(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> shaders =
	{
		{ "core/graphic/shaderSrc/cannyEdgeDetection/doubleThreshold/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/cannyEdgeDetection/doubleThreshold/fs.glsl", ShaderType::Fragment }
	};
	mProgram->buildFromFiles(shaders);
	std::initializer_list<FrameBufferSpecification> spec =
	{
		{
			AttachmentType::Color,
			TextureInternalFormat::RGB8,
			TextureDataFormat::RGB,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		}
	};
	mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, spec);
}

void DoubleThreshold::beginPass()
{
	RenderPass::beginPass();
	mProgram->setUniform("uLowThreshold", mLowThreshold);
	mProgram->setUniform("uHighThreshold", mHighThreshold);
	
	auto p = prev();
	Texture* tex = prev()->getCurrentFrameBuffer()->getColorAttachment(0);
	if (!tex)
	{
		KS_CORE_ERROR("framebuffer attachment at index {0} is null", 0);
		return;
	}
	tex->bind(0);
	mProgram->setUniform("nmsMap", 0);
}

void DoubleThreshold::runPass(Scene* scene)
{
	ScreenQuad* quad = scene->getScreenQuad();
	if (!quad)
	{
		KS_CORE_ERROR("pass screen quad is null");
		return;
	}
	quad->beginDraw();
	quad->draw();
	quad->endDraw();
}

void DoubleThreshold::setLowThreshold(float lowValue)
{
	mLowThreshold = lowValue;
}

float DoubleThreshold::getLowThreshold() const
{
	return mLowThreshold;
}

void DoubleThreshold::setHighThreshold(float highValue)
{
	mHighThreshold = highValue;
}

float DoubleThreshold::getHighThreshold() const
{
	return mHighThreshold;
}
