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
	if (mCurrentRenderPassGroup.empty())
	{
		setDefaultRenderPass();
	}
}

Renderer::~Renderer() {}

void Renderer::render()
{
	if (mCurrentRenderPassGroup.empty())
	{
		return;
	}

	renderUI();
	mCurrentScene->beginScene();	
	for (auto& pass : mCurrentRenderPassGroup)
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
	for (auto& pass : mCurrentRenderPassGroup)
	{
		pass->resize(width, height);
	}
}

//TODO:REMOVE
uint Renderer::getLastFrameBufferTexture() const
{
	return mCurrentRenderPassGroup.back()->getCurrentFrameBuffer()->getColorAttachment(0)->id();
}

//TODO REMOVE
int Renderer::getPassBufferTexture(RenderPassKey key)
{
	//TODO blurPass->getCurrentFrameBuffer()
	auto iter = mPassCache.find(key);
	if (iter != mPassCache.end() && iter->second.pass->isActive())
	{
		return iter->second.pass->getCurrentFrameBuffer()->getColorAttachment(0)->id();
	}
	return -1;
}

FrameBuffer* Renderer::getFrameBuffer(RenderPassKey key) const
{
	auto iter = mPassCache.find(key);
	if (iter != mPassCache.end() && iter->second.pass->isActive())
	{
		return iter->second.pass->getCurrentFrameBuffer();
	}
	else
	{
		return nullptr;
	}	
}

RenderPass* Renderer::getRenderPass(RenderPassKey key) const
{
	auto iter = mPassCache.find(key);
	if (iter != mPassCache.end())
	{
		return iter->second.pass.get();
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

void Renderer::setEnableBloom(bool enable)
{	
	if (mRenderPipeLine.enableBloom != enable)
	{
		mRenderPipeLine.enableBloom = enable;
		//add or remove bloom pass and gaussian pass
		if (enable)
		{
			auto lastPass1 = *(--(--mCurrentRenderPassGroup.end()));
			addRenderPass(RenderPassKey::BLOOM, RenderState{ mViewport }, lastPass1);

			auto lastPass2 = *(--(--mCurrentRenderPassGroup.end()));
			addRenderPass(RenderPassKey::BLOOMBLUR, RenderState{ mViewport }, lastPass2);
		}
		else
		{
			RenderPass* prev{nullptr};
			RenderPass* next{nullptr};
			RenderPass* bloomPass{nullptr};
			RenderPass* bloomBlurPass{nullptr};
			auto bloomIter = mPassCache.find(RenderPassKey::BLOOM);
			if (bloomIter != mPassCache.end())
			{
				bloomIter->second.pass->deActive();
				bloomPass = bloomIter->second.pass.get();
				if (bloomPass)
				{					
					prev = bloomPass->prev();
				}
			}

			auto bloomBlurIter = mPassCache.find(RenderPassKey::BLOOMBLUR);
			if (bloomBlurIter != mPassCache.end())
			{
				bloomBlurIter->second.pass->deActive();
				bloomBlurPass = bloomBlurIter->second.pass.get();
				if (bloomBlurPass)
				{
					next = bloomBlurPass->next();
				}
			}

			if (prev && next)
			{
				prev->setNext(next);
				next->setPrev(prev);
				mCurrentRenderPassGroup.remove(bloomPass);
				mCurrentRenderPassGroup.remove(bloomBlurPass);
			}
		}
	}
}

bool Renderer::getEnableBloom() const
{
	return mRenderPipeLine.enableBloom;
}

uint Renderer::getBloomBlur() const
{
	return mRenderPipeLine.bloomBlur;
}

void Renderer::setBloomBlur(uint blur)
{
	if (mRenderPipeLine.bloomBlur != blur)
	{
		mRenderPipeLine.bloomBlur = blur;
		auto Pass = mPassCache.find(RenderPassKey::BLOOMBLUR);
		GaussianBlur* blurPass = static_cast<GaussianBlur*>(Pass->second.pass.get());
		blurPass->setBloomBlur(blur);
	}
}

void Renderer::setDefaultRenderPass()
{
	//TODO: default renderpass forwardShading + toneMapping
	auto gPass = addRenderPass(RenderPassKey::GEOMETRY, RenderState{ mViewport }, nullptr);
	auto lightingPass =addRenderPass(RenderPassKey::DEFFEREDSHADING, RenderState{ mViewport }, gPass);
	auto toneMappingPass = addRenderPass(RenderPassKey::TONEMAPPING, RenderState{ mViewport }, lightingPass);
}

RenderPass* Renderer::addRenderPass(RenderPassKey key, const RenderState& state, RenderPass* where)
{
	auto passCacheIter = mPassCache.find(key);
	std::shared_ptr<RenderPass> currentPass;
	if (passCacheIter == mPassCache.cend())
	{
		currentPass = PassFactory::ceate(this, key, state);	
		mPassCache.insert({ key, RenderKeyPass{key, currentPass} });
	}
	else if (passCacheIter != mPassCache.cend())
	{
		currentPass = passCacheIter->second.pass;
		passCacheIter->second.pass->active();
	}
	
	auto currentPassIter = std::find(mCurrentRenderPassGroup.begin(), mCurrentRenderPassGroup.end(), currentPass.get());
	
	if (currentPassIter == mCurrentRenderPassGroup.end())
	{
		if (where)
		{
			auto next = where->next();
			if (next)
			{
				next->setPrev(currentPass.get());
				currentPass->setNext(next);
			}

			auto whereIter = std::find(mCurrentRenderPassGroup.begin(), mCurrentRenderPassGroup.end(), where);
			currentPass->setPrev(where);
			where->setNext(currentPass.get());
			
			mCurrentRenderPassGroup.insert(++whereIter, currentPass.get());
		}
		else 
		{
			mCurrentRenderPassGroup.push_back(currentPass.get());
		}
		return currentPass.get();
	}
	return nullptr;
}

void Renderer::removePass(RenderPassKey key)
{
}

void Renderer::renderUI()
{
	ImGui::Begin("Graphic Settings");

	//POM
	bool pomChecked = mRenderPipeLine.enableParallaxOcclusion;
	if (ImGui::Checkbox("Parallax Occlusion", &pomChecked))
	{
		enableParallexOcclusion(pomChecked);
	}
	if (pomChecked)
	{
		float pomScale = mRenderPipeLine.parallaxOcclusionScale;
		if (ImGui::DragFloat("Parallax Occlusion Scale", &pomScale, 0.001, 0.0, 0.05))
		{
			setPOMScale(pomScale);
		}
	}
	ImGui::Separator();

	//BLOOM
	bool bloomChecked = mRenderPipeLine.enableBloom;
	if (ImGui::Checkbox("Bloom", &bloomChecked))
	{
		setEnableBloom(bloomChecked);
	}

	if (bloomChecked)
	{
		int bloomBlur = mRenderPipeLine.bloomBlur;
		if (ImGui::DragInt("Bloom Blur", &bloomBlur, 1, 0, 12))
		{
			setBloomBlur(bloomBlur);
		}
	}
	ImGui::Separator();

	ImGui::End();
}
