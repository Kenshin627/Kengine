#include "lightingPass.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"
#include "graphic/gpuBuffer/frameBuffer.h"

LightingPass::LightingPass(const RenderState& state)
	:RenderPass(state)
{
	//to screen no fbo
	mProgram = std::make_unique<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/deferredRendering/lightingPassShader/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/deferredRendering/lightingPassShader/fs.glsl", ShaderType::Fragment }
	});

	mGeometry = std::make_shared<ScreenQuad>();
}

void LightingPass::runPass(Scene* scene)
{	
	mGeometry->draw();
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
