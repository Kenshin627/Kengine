#include <glad/glad.h>
#include "scene/scene.h"
#include "renderer.h"
#include "scene/camera/camera.h"
#include "graphic/renderPass/renderPass.h"
#include "graphic/renderPass/forwardShading/forwardShadingPass.h"
#include "graphic/gpuBuffer/frameBuffer.h"
#include "graphic/renderPass/gaussianBlur/gaussianBlur.h"
#include "graphic/renderPass/deferredRendering/geometryPass.h"
#include "graphic/renderPass/deferredRendering/lightingPass.h"
#include "graphic/renderPass/toneMapping/toneMapping.h"
#include "graphic/renderPass/passFactory.h"
#include "imgui.h"

Renderer::Renderer(uint width, uint height)
	:mWidth(width),
	 mHeight(height),
	 mViewport(0, 0, width, height)
{
	if (mRenderPasses.empty())
	{
		setDefaultRenderPass();
	}
}

Renderer::~Renderer() {}

void Renderer::render()
{
	if (mRenderPasses.empty())
	{
		return;
	}

	renderUI();
	mCurrentScene->beginScene();	
	//run pass loop
	for (auto& pass : mRenderPasses)
	{
		pass.second->renderUI();
		pass.second->beginPass();
		pass.second->runPass(mCurrentScene.get());
		pass.second->endPass();
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
		pass.second->resize(width, height);
	}
}

uint Renderer::getLastFrameBufferTexture() const
{
	return mRenderPasses.rbegin()->second->getCurrentFrameBuffer()->getColorAttachment(0)->id();
}

//TODO REMOVE
int Renderer::getPassBufferTexture(RenderPassKey key)
{
	//TODO blurPass->getCurrentFrameBuffer()
	auto iter = mPassCache.find(key);
	if (iter != mPassCache.end())
	{
		return iter->second->getCurrentFrameBuffer()->getColorAttachment(0)->id();
	}
	return -1;
}

FrameBuffer* Renderer::getFrameBuffer(RenderPassKey key) const
{
	auto iter = mRenderPasses.find(key);
	if (iter != mRenderPasses.end())
	{
		return iter->second->getCurrentFrameBuffer();
	}
	else
	{
		return nullptr;
	}	
}

RenderPass* Renderer::getRenderPass(RenderPassKey key) const
{
	auto iter = mRenderPasses.find(key);
	if (iter != mRenderPasses.end())
	{
		return iter->second;
	}
	return nullptr;
}

void Renderer::enableParallexOcclusion(bool enable)
{
	if (mRenderPipeLine.enableParallaxOcclusion != enable)
	{
		mRenderPipeLine.enableParallaxOcclusion = enable;
	}
}

bool Renderer::getParallaxOcclusion() const
{
	return mRenderPipeLine.enableParallaxOcclusion;
}

float Renderer::getPOMScale() const
{
	return mRenderPipeLine.parallaxOcclusionScale;
}

void Renderer::setPOMScale(float scale)
{
	mRenderPipeLine.parallaxOcclusionScale = scale;
}

void Renderer::setDefaultRenderPass()
{
	//TODO: default renderpass forwardShading + toneMapping
	addRenderPass(RenderPassKey::GEOMETRY, RenderState{ mViewport });
	addRenderPass(RenderPassKey::DEFFEREDSHADING, RenderState{ mViewport });
	addRenderPass(RenderPassKey::TONEMAPPING, RenderState{ mViewport });
}

void Renderer::addRenderPass(RenderPassKey key, const RenderState& state)
{
	auto passCacheIter = mPassCache.find(key);
	if (passCacheIter == mPassCache.cend())
	{
		auto currentPass = PassFactory::ceate(this, key, state);
		if (!mRenderPasses.empty())
		{
			RenderPass* lastPass = mRenderPasses.rbegin()->second;
			if (lastPass)
			{
				lastPass->setNext(currentPass.get());
				currentPass->setPrev(lastPass);
			}
		}
		
		mRenderPasses.insert({ key, currentPass.get() });
		mPassCache.insert({ key, std::move(currentPass) });
	}
	if (passCacheIter != mPassCache.cend())
	{
		RenderPass* currentPass = passCacheIter->second.get();
		auto currentPassIter = mRenderPasses.find(key);
		if (currentPassIter == mRenderPasses.cend())
		{
			RenderPass* lastPass = mRenderPasses.rbegin()->second;
			if (!lastPass)
			{
				lastPass->setNext(currentPass);
				currentPass->setPrev(lastPass);
			}
			mRenderPasses.insert({ key, currentPass });
		}
	}
}

void Renderer::renderUI()
{
	ImGui::Begin("Graphic Settings");

	bool pomChecked = mRenderPipeLine.enableParallaxOcclusion;
	if (ImGui::Checkbox("Parallax Occlusion", &pomChecked))
	{
		mRenderPipeLine.enableParallaxOcclusion = pomChecked;
	}
	if (pomChecked)
	{
		float pomScale = mRenderPipeLine.parallaxOcclusionScale;
		if (ImGui::DragFloat("Parallax Occlusion Scale", &pomScale, 0.001, 0.0, 0.05))
		{
			mRenderPipeLine.parallaxOcclusionScale = pomScale;
		}
	}
	ImGui::Separator();


	ImGui::End();
}
