#include "core.h"
#include "bloomPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include <graphic/program/program.h>
#include "geometry/screenQuad.h"
#include "scene/scene.h"

BloomPass::BloomPass(const RenderState& state)
	:RenderPass(state)
{
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> shaders =
	{
		{ "core/graphic/shaderSrc/bloom/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/bloom/fs.glsl", ShaderType::Fragment }
	};
	mProgram->buildFromFiles(shaders);
	std::initializer_list<FrameBufferSpecification> specs =
	{
		//color attachment 0 normal ldr
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA16F,
			TextureDataFormat::RGB,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		},
		//color attachmeng 1 hdr
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA16F,
			TextureDataFormat::RGB,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		}
	};
	mFrameBuffer = std::make_shared<FrameBuffer>(state.width, state.height, specs);
}

BloomPass::~BloomPass()
{
}

void BloomPass::beginPass()
{
	RenderPass::beginPass();
	Texture2D* tex = mlastPassFrameBuffer[0]->getColorAttachment(0);
	tex->bind();
	mProgram->setUniform("screenMap", 0);
}

void BloomPass::runPass(Scene* scene)
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
