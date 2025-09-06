#include <glad/glad.h>
#include "scene/scene.h"
#include "renderer.h"
#include "scene/camera/camera.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::clear()
{
	//TODO: set in pass
	glClear(mClearBits);
	glClearColor(mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a);
	glClearDepth(mClearDepth);
}

void Renderer::setViewport(const glm::vec4& viewport)
{
	mViewport = viewport;
}

void Renderer::viewPort() const
{
	//TODO: set in pass
	glViewport(mViewport[0], mViewport[1], mViewport[2], mViewport[3]);
}

void Renderer::setClearColor(const glm::vec4& c)
{
	setClearColor(c.r, c.g, c.b, c.a);
}

void Renderer::setClearColor(float r, float g, float b, float a)
{
	mClearColor.r = r;
	mClearColor.g = g;
	mClearColor.b = b;
	mClearColor.a = a;
}

void Renderer::setClearDepth(double d)
{
	mClearDepth = d;
}

void Renderer::render()
{
	mCurrentScene->beginScene();
	// Render Passes
	mCurrentScene->draw();
	mCurrentScene->endScene();
}

void Renderer::onWindowSizeChanged(uint width, uint height)
{
	//set viewport
	setViewport(glm::vec4(mViewport[0], mViewport[1], width, height));
	//set camera aspect ratio
	if (mCurrentScene)
	{
		auto camera = mCurrentScene->getCurrentCamera();
		if (camera)
		{
			camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
		}
	}
}
