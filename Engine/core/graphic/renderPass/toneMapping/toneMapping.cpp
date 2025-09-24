#include "graphic/renderPass/bloomPass/bloomPass.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "toneMapping.h"
#include "scene/scene.h"
#include "core.h"
#include <graphic/renderPass/gaussianBlur/gaussianBlur.h>

ToneMapping::ToneMapping(float exposure, Renderer* r, const RenderState& state)
	:RenderPass(r, state),
	mExposure(exposure)
{
	mProgram = std::make_shared<Program>();
	std::initializer_list<ShaderFile> shaders =
	{
		{ "core/graphic/shaderSrc/toneMapping/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/toneMapping/fs.glsl", ShaderType::Fragment }
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

ToneMapping::~ToneMapping()
{
}

void ToneMapping::beginPass()
{
	RenderPass::beginPass();
	mProgram->setUniform("exposure", mExposure);
	
	auto pass = mOwner->getRenderPass(RenderPassKey::BLOOM);
	bool enableBloom = pass && (pass->isActive());
	mProgram->setUniform("enableBloom", enableBloom);
	Texture* ldrMap = nullptr;
	if (enableBloom)
	{	
		auto gaussianPass = static_cast<GaussianBlur*>(mOwner->getRenderPass(RenderPassKey::BLOOMBLUR));
		Texture* hdrBlurTex = gaussianPass->getOutputFrameBuffer()->getColorAttachment(0);
		hdrBlurTex->bind(1);
		mProgram->setUniform("hDRBlurMap", 1);
		auto bloomPass = static_cast<BloomPass*>(pass);
		ldrMap = bloomPass->getLDRTexture();		
	}
	else
	{
		auto ldrPass = mOwner->getRenderPass(RenderPassKey::DEFFEREDSHADING);
		ldrMap = ldrPass->getCurrentFrameBuffer()->getColorAttachment(0);
	}
	if (ldrMap)
	{
		ldrMap->bind(0);
		mProgram->setUniform("lDRMap", 0);
	}
}

void ToneMapping::runPass(Scene* scene)
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
