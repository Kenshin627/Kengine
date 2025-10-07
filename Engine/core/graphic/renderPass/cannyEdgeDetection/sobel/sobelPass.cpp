#include "sobelPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "scene/scene.h"
#include "core.h"
#include <graphic/renderPass/gaussianBlur/gaussianBlur.h>

SobelPass::SobelPass(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	mProgram = std::make_shared<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/cannyEdgeDetection/sobel/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/cannyEdgeDetection/sobel/fs.glsl", ShaderType::Fragment }
		});
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

void SobelPass::beginPass()
{
	RenderPass::beginPass();
	GaussianBlur* gaussianBlur = static_cast<GaussianBlur*>(prev());
	auto texture = gaussianBlur->getOutputFrameBuffer()->getColorAttachment(0);
	texture->bind(0);
	mProgram->setUniform("screenMap", 0);
}

void SobelPass::runPass(Scene* scene)
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
