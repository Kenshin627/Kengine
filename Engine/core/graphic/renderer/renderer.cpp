#include <glad/glad.h>
#include "scene/scene.h"
#include "renderer.h"
#include "scene/camera/camera.h"
#include "graphic/renderPass/renderPass.h"
#include "graphic/renderPass/defaultToScreen/defaultPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderPass/gaussianBlur/gaussianBlur.h"
Renderer::Renderer(uint width, uint height)
	:mWidth(width),
	 mHeight(height)
{

}

Renderer::~Renderer() {}

void Renderer::render()
{
	mCurrentScene->beginScene();
	if (mRenderPasses.empty())
	{
		setDefaultRenderPass();
	}

	//run pass loop
	for (auto& pass : mRenderPasses)
	{
		pass->renderUI();
		pass->beginPass();
		pass->runPass(mCurrentScene.get());
		pass->endPass();
	}
	
	mCurrentScene->endScene();
}

Scene* Renderer::getCurrentScene()
{
	return mCurrentScene.get();
}

void Renderer::onWindowSizeChanged(uint width, uint height)
{
	mWidth = width;
	mHeight = height;
	//set camera aspect ratio
	if (mCurrentScene)
	{
		auto camera = mCurrentScene->getCurrentCamera();
		if (camera)
		{
			//set dirty, next time update ubo
			camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
		}
	}
	//update renderPass, because renderPass maybe include fbo & viewport resize 
	for (auto& pass : mRenderPasses)
	{
		pass->resize(width, height);
	}
}

void Renderer::setRenderPass(const std::initializer_list<std::shared_ptr<RenderPass>>& passes)
{
	if (passes.size() == 0)
	{
		return;
	}
	mRenderPasses.clear();
	for (auto& pass : passes)
	{
		mRenderPasses.push_back(pass);
	}
}

uint Renderer::getLastFrameBufferTexture() const
{
	return mRenderPasses.back()->getCurrentFrameBuffer()->getColorAttachment(0)->id();
}

//TODO:PASS KEY
uint Renderer::getPassBufferTexture()
{
	//TODO blurPass->getCurrentFrameBuffer()
	return mRenderPasses.at(3).get()->getCurrentFrameBuffer()->getColorAttachment(0)->id();
}

void Renderer::setDefaultRenderPass()
{
	//set renderState
	RenderState state;
	state.width = mWidth;
	state.height = mHeight;
	state.viewport.z = mWidth;
	state.viewport.w = mHeight;
	state.target = RenderTarget::SCREEN;
	setRenderPass({ std::make_shared<DefaultPass>(state) });
}
