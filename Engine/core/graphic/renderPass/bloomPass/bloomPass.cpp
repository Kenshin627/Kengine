#include "core.h"
#include "bloomPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include <graphic/program/program.h>
#include "geometry/screenQuad.h"
#include "scene/scene.h"
#include "graphic/texture/texture.h"

BloomPass::BloomPass(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	mProgram = std::make_shared<Program>();

	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/bloom/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/bloom/fs.glsl", ShaderType::Fragment }
		});
	std::initializer_list<FrameBufferSpecification> specs =
	{
		//color attachment 0 normal ldr
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA16F,
			TextureDataFormat::RGBA,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		},
		//color attachmeng 1 hdr
		{
			AttachmentType::Color,
			TextureInternalFormat::RGBA16F,
			TextureDataFormat::RGBA,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureWarpMode::CLAMP_TO_EDGE,
			TextureFilter::NEAREST,
			TextureFilter::NEAREST
		}
	};
	mFrameBuffer = std::make_shared<FrameBuffer>(glm::vec3{ mSize.x, mSize.y, 0 }, specs);
}

BloomPass::~BloomPass()
{
}

void BloomPass::beginPass()
{
	RenderPass::beginPass();
	Texture* tex = mPrevPass->getCurrentFrameBuffer()->getColorAttachment(0);
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

Texture* BloomPass::getHDRTexture() const
{
	return mFrameBuffer->getColorAttachment(1);
}

Texture* BloomPass::getLDRTexture() const
{
	return mFrameBuffer->getColorAttachment(0);
}