#include "nonMaxSuppression.h"
#include "graphic/program/program.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "core.h"
#include "geometry/screenQuad.h"
#include "scene/scene.h"

NonMaxSuppression::NonMaxSuppression(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	mProgram = std::make_shared<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/cannyEdgeDetection/nms/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/cannyEdgeDetection/nms/fs.glsl", ShaderType::Fragment }
		});
	std::initializer_list<FrameBufferSpecification> spec =
	{
		{
			AttachmentType::Color,
			TextureInternalFormat::R8,
			TextureDataFormat::R,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		}
	};
	mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y ,0 }, spec);
	mRenderPassKey = RenderPassKey::NMS;
}

void NonMaxSuppression::beginPass()
{
	RenderPass::beginPass();
	//get sobel texture
	
	Texture* tex = prev()->getCurrentFrameBuffer()->getColorAttachment(0);
	if (!tex)
	{
		KS_CORE_ERROR("framebuffer attachment at index {0} is null", 0);
		return;
	}
	tex->bind(0);
	
	mProgram->setUniform("uGradientTexture", 0);
}

void NonMaxSuppression::runPass(Scene* scene)
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
