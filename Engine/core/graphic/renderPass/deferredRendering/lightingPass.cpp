#include "lightingPass.h"
#include "core.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "scene/scene.h"

LightingPass::LightingPass(const RenderState& state)
	:RenderPass(state)
{
	mProgram = std::make_unique<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/deferredRendering/lightingPassShader/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/deferredRendering/lightingPassShader/fs.glsl", ShaderType::Fragment }
	});
}

void LightingPass::runPass(Scene* scene)
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

void LightingPass::beginPass()
{
	//binding program first
	RenderPass::beginPass();
	//get colorattachment from gpass
	//set uniform textureSampler2D
	std::shared_ptr<FrameBuffer> geometryBuffer = mlastPassFrameBuffer[0];
	auto gPos = geometryBuffer->getColorAttachment(0);
	auto gNormal = geometryBuffer->getColorAttachment(1);
	auto gDiff = geometryBuffer->getColorAttachment(2);
	auto gSpecShiness = geometryBuffer->getColorAttachment(3);
	gPos->bind(0);
	gNormal->bind(1);
	gDiff->bind(2);
	gSpecShiness->bind(3);

	mProgram->setUniform("gPosition", 0);
	mProgram->setUniform("gNormal", 1);
	mProgram->setUniform("gDiffuse", 2);
	mProgram->setUniform("gSpecShiness", 3);

	std::shared_ptr<FrameBuffer> blurSsaoBuffer = mlastPassFrameBuffer[1];
	auto blurSsao = blurSsaoBuffer->getColorAttachment(0);
	blurSsao->bind(4);
	mProgram->setUniform("ssaoMap", 4);
}
