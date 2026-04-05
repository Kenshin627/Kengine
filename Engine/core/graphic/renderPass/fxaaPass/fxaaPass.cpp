#include "graphic/renderPass/bloomPass/bloomPass.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "fxaaPass.h"
#include "scene/scene.h"
#include "core.h"
#include <graphic/renderPass/gaussianBlur/gaussianBlur.h>

FXAA::FXAA(Renderer* r, const RenderState& state)
	:RenderPass(r, state)
{
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> shaders =
	{
		{ "core/graphic/shaderSrc/fxaa/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/fxaa/fs.glsl", ShaderType::Fragment }
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
		},
		//debugView
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
	mRenderPassKey = RenderPassKey::FXAA;
}

FXAA::~FXAA()
{
}

void FXAA::beginPass()
{
	RenderPass::beginPass();	
	auto pass = mOwner->getRenderPass(RenderPassKey::TONEMAPPING);
	Texture* tex = pass->getCurrentFrameBuffer()->getColorAttachment(0);
	tex->bind(0);
	glm::vec2 texelSize = glm::vec2(1.0f) / glm::vec2(tex->width(), tex->height());
	mProgram->setUniform("screenMap", 0);
	mProgram->setUniform("texelSize", texelSize);
	mProgram->setUniform("fxaaConfig", mFxaaConfig);
	mProgram->setUniform("fxaaQuality", 1);
}

void FXAA::runPass(Scene* scene)
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

Texture* FXAA::getDebugView() const
{
	return mFrameBuffer->getColorAttachment(1);
}

void FXAA::setFixThreshold(float threshold)
{
	mFxaaConfig.x = threshold;
}

void FXAA::setRelativeThreshold(float threshold)
{
	mFxaaConfig.y = threshold;
}

void FXAA::setBlendFactor(float factor)
{
	mFxaaConfig.z = factor;
}

float FXAA::getFixTreshold() const
{
	return mFxaaConfig.x;
}

float FXAA::getRelativeThreshold() const
{
	return mFxaaConfig.y;
}

float FXAA::getBlendFactor() const
{
	return mFxaaConfig.z;
}
