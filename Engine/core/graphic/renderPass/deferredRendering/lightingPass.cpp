#include "lightingPass.h"
#include "graphic/program/program.h"
#include "geometry/screenQuad.h"

LightingPass::LightingPass(uint width, uint height)
{
	//to screen no fbo
	mProgram = std::make_unique<Program>();
	mProgram->buildFromFiles({
		{ "core/graphic/shaderSrc/deferredRendering/lightingPassShader/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/deferredRendering/lightingPassShader/fs.glsl", ShaderType::Fragment }
	});

	mGeometry = std::make_shared<ScreenQuad>();

	//set renderState
	RenderState state;
	state.width = width;
	state.height = height;
	state.viewport.z = width;
	state.viewport.w = height;
	state.target = RenderTarget::SCREEN;
	setRenderState(state);
}
