#include "core.h"
#include "grayScaleEffect.h"
#include "geometry/screenQuad.h"
#include "graphic/program/program.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "scene/scene.h"

GrayScaleEffect::GrayScaleEffect(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	mProgram = std::make_shared<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/postProcess/grayScale/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/postProcess/grayScale/fs.glsl", ShaderType::Fragment }
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
	mRenderPassKey = RenderPassKey::GRAYSCALER;
}

void GrayScaleEffect::runPass(Scene* scene)
{
	//get screenMap texuture
	if (!mPrevPass)
	{
		//TODO
		Texture* tex = prev()->getCurrentFrameBuffer()->getColorAttachment(0);
		if (!tex)
		{
			KS_CORE_ERROR("framebuffer attachment at index {0} is null", 0);
			return;
		}
		tex->bind(0);
	}
	//set screen map uniform
	mProgram->setUniform("screenMap", 0);
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
