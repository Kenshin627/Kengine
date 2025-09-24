#include "graphic/renderPass/bloomPass/bloomPass.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderer/renderer.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "toneMapping.h"
#include "scene/scene.h"
#include "core.h"

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
	Texture* prevLightingTex = mPrevPass->getCurrentFrameBuffer()->getColorAttachment(0);
	prevLightingTex->bind(0);
	mProgram->setUniform("prevLightingMap", 0);	
	auto pass = mOwner->getRenderPass(RenderPassKey::BLOOM);
	mProgram->setUniform("enableBloom", pass != nullptr);
	if (pass)
	{	
		auto bloomPass = static_cast<BloomPass*>(pass);
		Texture* ldrMap = bloomPass->getLDRTexture();
		ldrMap->bind(1);
		mProgram->setUniform("lDRMap", 1);
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
