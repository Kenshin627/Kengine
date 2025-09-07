#include "defaultPass.h"
#include "scene/scene.h"

DefaultPass::DefaultPass(uint width, uint height)
{
	//default to screen, no fbo, no program, no vbo
	//set renderState
	//width height toScreen
	RenderState state;
	state.width = width;
	state.height = height;
	state.viewport.z = width;
	state.viewport.w = height;
	state.target = RenderTarget::SCREEN;
	setRenderState(state);
}

DefaultPass::~DefaultPass()
{
}

void DefaultPass::runPass(Scene* scene)
{
	scene->draw();
}
