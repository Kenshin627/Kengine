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
}

ToneMapping::~ToneMapping()
{
}

void ToneMapping::beginPass()
{
	RenderPass::beginPass();
	mProgram->setUniform("exposure", mExposure);
	Texture2D* tex = mlastPassFrameBuffer[0]->getColorAttachment(0);
	tex->bind(0);
	mProgram->setUniform("screenMap", 0);
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
