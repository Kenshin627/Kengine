#include "toneMapping.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "core.h"
#include "scene/scene.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "graphic/gpuBuffer/frameBuffer.h"

ToneMapping::ToneMapping(float exposure, const RenderState& state)
	:RenderPass(state),
	mExposure(exposure)
{
	//program
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
			TextureFilter::LINEAR,
			TextureFilter::LINEAR
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
	Texture* gaussianBlurMap = mlastPassFrameBuffer[0]->getColorAttachment(0);
	gaussianBlurMap->bind(0);
	mProgram->setUniform("gaussian", 0);

	Texture* ldrMap = mlastPassFrameBuffer[1]->getColorAttachment(0);
	ldrMap->bind(1);
	mProgram->setUniform("ldrMap", 1);
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
