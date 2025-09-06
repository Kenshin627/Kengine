#include <glad/glad.h>
#include "scene/scene.h"
#include "renderer.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::clear(float r, float g, float b, float a)
{
	//TODO: set in pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(r, g, b, a);
}

void Renderer::setViewport(int x, int y, int width, int height)
{
	//TODO: set in pass
	mViewport[0] = x;
	mViewport[1] = y;
	mViewport[2] = width;
	mViewport[3] = height;
	glViewport(x, y, width, height);
}

void Renderer::render()
{
	mCurrentScene->beginScene();
	// Render Passes
	mCurrentScene->draw();
	mCurrentScene->endScene();
}
