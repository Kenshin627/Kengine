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
#include "implot.h"
#include "implot_internal.h"
#include <deque>

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
			addRenderPass(RenderPassKey::BLOOM, RenderState{ mViewport, false }, lastPass1);

			auto lastPass2 = *(--(--mCurrentRenderPassGroup.end()));
			addRenderPass(RenderPassKey::BLOOMBLUR, RenderState{ mViewport, false }, lastPass2);
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
	RenderPass* pass = getRenderPass(RenderPassKey::BLOOMBLUR);
	if (!pass)
	{
		return 0;
	}
	GaussianBlur* gaussianPass = static_cast<GaussianBlur*>(pass);
	return gaussianPass->getGussianBlurAmount();
}

void Renderer::setBloomBlur(uint blur)
{
	RenderPass* pass = getRenderPass(RenderPassKey::BLOOMBLUR);
	GaussianBlur* gaussianPass = static_cast<GaussianBlur*>(pass);
	if (gaussianPass->getGussianBlurAmount() != blur)
	{
		gaussianPass->setGaussianBlurAmount(blur);
	}
}

float Renderer::getBloomBlurScale()
{
	RenderPass* pass = getRenderPass(RenderPassKey::BLOOMBLUR);
	if (!pass)
	{
		return 0;
	}
	GaussianBlur* gaussianPass = static_cast<GaussianBlur*>(pass);
	return gaussianPass->getGussianBlurScale();
}

void Renderer::setBloomBlurScale(float s)
{
	RenderPass* pass = getRenderPass(RenderPassKey::BLOOMBLUR);
	if (!pass)
	{
		return;
	}
	GaussianBlur* gaussianPass = static_cast<GaussianBlur*>(pass);
	if (gaussianPass->getGussianBlurScale() != s)
	{
		gaussianPass->setGaussianBlurScale(s);
	}
}

float Renderer::getBloomBlurStrength()
{
	RenderPass* pass = getRenderPass(RenderPassKey::BLOOMBLUR);
	if (!pass)
	{
		return 0;
	}
	GaussianBlur* gaussianPass = static_cast<GaussianBlur*>(pass);
	return gaussianPass->getGussianBlurStrength();
}

void Renderer::setBloomBlurStrength(float s)
{
	RenderPass* pass = getRenderPass(RenderPassKey::BLOOMBLUR);
	GaussianBlur* gaussianPass = static_cast<GaussianBlur*>(pass);
	if (gaussianPass->getGussianBlurStrength() != s)
	{
		gaussianPass->setGaussianBlurStrength(s);
	}
}

void Renderer::enableSSAO(bool enable)
{
	if (mRenderPipeLine.enableSSao != enable)
	{
		mRenderPipeLine.enableSSao = enable;
		if (enable)
		{
			RenderPass* gPass = getRenderPass(RenderPassKey::GEOMETRY);
			RenderPass* ssao = addRenderPass(RenderPassKey::SSAO, RenderState{ mViewport, false }, gPass);
			addRenderPass(RenderPassKey::SSAOBLUR, RenderState{ mViewport, false }, ssao);
		}
		else
		{
			RenderPass* prev{ nullptr };
			RenderPass* next{ nullptr };
			RenderPass* ssaoPass{ nullptr };
			RenderPass* ssaoBlurPass{ nullptr };
			auto ssaoIter = mPassCache.find(RenderPassKey::SSAO);
			if (ssaoIter != mPassCache.end())
			{
				ssaoIter->second.pass->deActive();
				ssaoPass = ssaoIter->second.pass.get();
				if (ssaoPass)
				{
					prev = ssaoPass->prev();
				}
			}

			auto ssaoBlurIter = mPassCache.find(RenderPassKey::SSAOBLUR);
			if (ssaoBlurIter != mPassCache.end())
			{
				ssaoBlurIter->second.pass->deActive();
				ssaoBlurPass = ssaoBlurIter->second.pass.get();
				if (ssaoBlurPass)
				{
					next = ssaoBlurPass->next();
				}
			}

			if (prev && next)
			{
				prev->setNext(next);
				next->setPrev(prev);
				mCurrentRenderPassGroup.remove(ssaoPass);
				mCurrentRenderPassGroup.remove(ssaoBlurPass);
			}
		}
	}
}

bool Renderer::getEnableSSAO() const
{
	return mRenderPipeLine.enableSSao;
}

uint Renderer::getSSAOKernelSize() const
{
	RenderPass* pass = getRenderPass(RenderPassKey::SSAO);
	if (!pass)
	{
		return 0;
	}
	SSAOPass* ssao = static_cast<SSAOPass*>(pass);
	return ssao->getKernelSize();
}

void Renderer::setSSAOKernelSize(uint kernelSize)
{
	RenderPass* pass = getRenderPass(RenderPassKey::SSAO);
	SSAOPass* ssao = static_cast<SSAOPass*>(pass);
	ssao->setKernelSize(kernelSize);
}

float Renderer::getSSAOBias() const
{
	RenderPass* pass = getRenderPass(RenderPassKey::SSAO);
	if (!pass)
	{
		return 0;
	}
	SSAOPass* ssao = static_cast<SSAOPass*>(pass);
	return getSSAOBias();
}

void Renderer::setSSAOBias(float bias)
{
	RenderPass* pass = getRenderPass(RenderPassKey::SSAO);
	SSAOPass* ssao = static_cast<SSAOPass*>(pass);
	ssao->setBias(bias);
}

uint Renderer::getSSAOBlurRadius() const
{
	RenderPass* pass = getRenderPass(RenderPassKey::SSAOBLUR);
	if (!pass)
	{
		return 0;
	}
	BlurPass* blurPass = static_cast<BlurPass*>(pass);
	return blurPass->getBlurRadius();
}

void Renderer::setSSAOBlurRadius(float blurRadius)
{
	RenderPass* pass = getRenderPass(RenderPassKey::SSAOBLUR);
	BlurPass* blurPass = static_cast<BlurPass*>(pass);
	blurPass->setBlurRadius(blurRadius);
}

uint Renderer::getSSAOSamplerRadius() const
{
	RenderPass* pass = getRenderPass(RenderPassKey::SSAO);
	if (!pass)
	{
		return 0;
	}
	SSAOPass* ssao = static_cast<SSAOPass*>(pass);
	return ssao->getSamplerRadius();
}

void Renderer::setSSAOSamplerRadius(float samplerRadius)
{
	RenderPass* pass = getRenderPass(RenderPassKey::SSAO);
	SSAOPass* ssao = static_cast<SSAOPass*>(pass);
	ssao->setSamplerRadius(samplerRadius);
}

const DebugView& Renderer::getDebugView() const
{
	return mDebugView;
}

void Renderer::enableCSM(bool enable)
{
	if (mRenderPipeLine.enableCascadedShadowMap != enable)
	{
		mRenderPipeLine.enableCascadedShadowMap = enable;
		if (enable)
		{
			addRenderPass(RenderPassKey::CSM, RenderState{ mViewport, true }, nullptr);
		}
		else
		{
			RenderPass* prev{ nullptr };
			RenderPass* next{ nullptr };
			RenderPass* csmPass{ nullptr };
			auto csmIter = mPassCache.find(RenderPassKey::CSM);
			if (csmIter != mPassCache.end())
			{
				csmIter->second.pass->deActive();
				csmPass = csmIter->second.pass.get();
				if (csmPass)
				{
					prev = csmPass->prev();
					next = csmPass->next();
				}
			}			
			
			if (prev && next)
			{
				prev->setNext(next);
				next->setPrev(prev);
			}
			mCurrentRenderPassGroup.remove(csmPass);
		}
	}
}

bool Renderer::getEnableCSM() const
{
	return mRenderPipeLine.enableCascadedShadowMap;
}

void Renderer::setDefaultRenderPass()
{
	//TODO: default renderpass forwardShading + toneMapping
	auto gPass = addRenderPass(RenderPassKey::GEOMETRY, RenderState{ mViewport }, nullptr);
	auto lightingPass =addRenderPass(RenderPassKey::DEFFEREDSHADING, RenderState{ mViewport, false }, gPass);
	auto toneMappingPass = addRenderPass(RenderPassKey::TONEMAPPING, RenderState{ mViewport, false }, lightingPass);
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
			mCurrentRenderPassGroup.push_front(currentPass.get());
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
	if (!pomChecked)
	{
		ImGui::BeginDisabled();
	}
	float pomScale = mRenderPipeLine.parallaxOcclusionScale;
	if (ImGui::DragFloat("Parallax Occlusion Scale", &pomScale, 0.001, 0.0, 0.05))
	{
		setPOMScale(pomScale);
	}
	if (!pomChecked)
	{
		ImGui::EndDisabled();
	}
	ImGui::Separator();

	//BLOOM
	bool bloomChecked = mRenderPipeLine.enableBloom;
	if (ImGui::Checkbox("Bloom", &bloomChecked))
	{
		setEnableBloom(bloomChecked);
	}

	if (!bloomChecked)
	{
		ImGui::BeginDisabled();
	}
	int bloomBlur = getBloomBlur();
	if (ImGui::DragInt("Bloom Blur", &bloomBlur, 1, 0, 12))
		{
			setBloomBlur(bloomBlur);
		}

	float scale = getBloomBlurScale();
	if (ImGui::DragFloat("Bloom Scale", &scale, 0.01f, 0.1f, 6.0f))
		{
			setBloomBlurScale(scale);
		}

	float strength = getBloomBlurStrength();
	if (ImGui::DragFloat("Bloom Strength", &strength, 0.01f, 0.0f, 5.0f))
		{
			setBloomBlurStrength(strength);
		}

	//debugView
	ImGui::PushID("bloom");
	bool bloomDebugView = mRenderPipeLine.debugBloom;
	if (ImGui::Checkbox("DebugView", &bloomDebugView))
	{
		mRenderPipeLine.debugBloom = bloomDebugView;
		if (bloomDebugView)
		{
			mDebugView.colorAttachmentIndex = 0;
			mDebugView.fbo = static_cast<GaussianBlur*>(getRenderPass(RenderPassKey::BLOOMBLUR))->getOutputFrameBuffer();
			mDebugView.type = DebugViewAttachmentType::Color;
		}
		else
		{
			resetDebugView();
		}
	}
	ImGui::PopID();
	if(!bloomChecked)
	{
		ImGui::EndDisabled();
	
	}

	if(!bloomChecked)
	{
		if (mRenderPipeLine.debugBloom)
		{
			mRenderPipeLine.debugBloom = false;
			resetDebugView();
		}
	}
	ImGui::Separator();

	//SSAO
	bool ssaoChecked = mRenderPipeLine.enableSSao;
	if (ImGui::Checkbox("SSAO", &ssaoChecked))
	{
		enableSSAO(ssaoChecked);
	}

	if (!ssaoChecked)
	{
		ImGui::BeginDisabled();
	}
	SSAOPass* ssao = static_cast<SSAOPass*>(getRenderPass(RenderPassKey::SSAO));
	BlurPass* ssaoBlur = static_cast<BlurPass*>(getRenderPass(RenderPassKey::SSAOBLUR));
	
	//kernelSize
	int kernelSize = ssao? ssao->getKernelSize() : 0;
	if (ImGui::DragInt("KernelSize", &kernelSize, 1, 2, 1024))
	{
		ssao->setKernelSize(kernelSize);
	}

	//samplerRadius
	float samplerRadius = ssao? ssao->getSamplerRadius(): 0;
	if (ImGui::DragFloat("Sampler Radius", &samplerRadius, 0.01f, 0.01f, 12.0f))
	{
		ssao->setSamplerRadius(samplerRadius);
	}

	//bias
	float bias = ssao? ssao->getBias() : 0;
	if (ImGui::DragFloat("Bias", &bias, 0.001f, 0.001f, 1.0f))
	{
		ssao->setBias(bias);
	}

	//blurRadius
	int blurRadius = ssaoBlur? ssaoBlur->getBlurRadius():0;
	if (ImGui::DragInt("Blur Radius", &blurRadius, 1, 2, 64))
	{
		ssaoBlur->setBlurRadius(blurRadius);
	}

	// debugView
	ImGui::PushID("ssao");
	bool ssaodebugView = mRenderPipeLine.debugSSAO;
	if (ImGui::Checkbox("DebugView", &ssaodebugView))
	{
		mRenderPipeLine.debugSSAO = ssaodebugView;
		if (ssaodebugView)
		{
			mDebugView.colorAttachmentIndex = 0;
			mDebugView.fbo = getRenderPass(RenderPassKey::SSAO)->getCurrentFrameBuffer();
			mDebugView.type = DebugViewAttachmentType::Color;
		}
		else
		{
			resetDebugView();
		}
	}
	ImGui::PopID();
	if(!ssaoChecked)
	{
		ImGui::EndDisabled();
	}

	if(!ssaoChecked)
	{
		if (mRenderPipeLine.debugSSAO)
		{
			resetDebugView();
			mRenderPipeLine.debugSSAO = false;
		}
	}

	//CSM
	bool csmChecked = mRenderPipeLine.enableCascadedShadowMap;
	if (ImGui::Checkbox("Csm", &csmChecked))
	{
		enableCSM(csmChecked);
	}
	if (!csmChecked)
	{
		ImGui::BeginDisabled();
	}
	auto csm = static_cast<CascadeShadowMapPass*>(getRenderPass(RenderPassKey::CSM));
	int currentSplitMethodIndx = static_cast<int>(csm? csm->getSplitMethod(): FrustumSplitMethod::Uniform);
	if (ImGui::Combo(splitMethods[currentSplitMethodIndx], &currentSplitMethodIndx, splitMethods, IM_ARRAYSIZE(splitMethods))) {
		csm->selectSplitMethod(static_cast<FrustumSplitMethod>(currentSplitMethodIndx));
	}
	//display cascaded color
	bool showCsmColor = csm? csm->getDisplayCacadedColor():false;
	if (ImGui::Checkbox("display Cascaded Color", &showCsmColor))
	{
		csm->setDisplayCacadedColor(showCsmColor);
	}
	//splitLambda
	float splitLambda = csm? csm->getSplitLambda():1.0;
	if (ImGui::DragFloat("splitLambda", &splitLambda, 0.001, 0.0, 1.0))
	{
		csm->setSplitLambda(splitLambda);
	}

	//enable pcf or not
	bool enablePcf =  csm?csm->getEnablePCF():false;
	if (ImGui::Checkbox("PCF", &enablePcf))
	{
		csm->setEnablePCF(enablePcf);
	}

	//pcfSize
	if(!enablePcf)
	{
		ImGui::BeginDisabled();
	}
	int pcfSize = csm? csm->getPcfSize(): 0;
	if (ImGui::DragInt("PCFSize", &pcfSize, 1, 1, 16))
	{
		csm->setPcfSize(pcfSize);
	}
	if(!enablePcf)
	{
		ImGui::EndDisabled();
	}
	
	if(!csmChecked)
	{
		ImGui::EndDisabled();
	}
	ImGui::End();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	float current_fps = io.Framerate;

	static const int MAX_DATA_POINTS = 120;
	static std::deque<float> fps_data;

	// 2. 用于X轴的时间戳（可选，但推荐）
	// 或者我们可以直接使用数据点的索引作为X值
	static std::deque<float> time_data;
	static float elapsed_time = 0.0f;
	// 更新时间
	elapsed_time += io.DeltaTime;

	// 将新数据添加到缓冲区末尾
	fps_data.push_back(current_fps);
	time_data.push_back(elapsed_time);

	// 如果数据点数量超过最大值，移除最前面的旧数据
	if (fps_data.size() > MAX_DATA_POINTS) {
		fps_data.pop_front();
		time_data.pop_front();
	}

	ImGui::Begin("RenderStatus");
	ImPlot::SetNextAxesLimits(0, MAX_DATA_POINTS, 0, 500, ImGuiCond_Always);
	if (ImPlot::BeginPlot("FPS Curve")) 
	{
		ImPlot::SetupAxes("Frame", "FPS");
		std::vector<float> temp_fps_vector(fps_data.begin(), fps_data.end());
		ImPlot::PlotLine("FPS", temp_fps_vector.data(), fps_data.size());
		ImPlot::EndPlot();
	}
	ImGui::End();
}

void Renderer::resetDebugView()
{
	mDebugView.colorAttachmentIndex = -1;
	mDebugView.fbo = nullptr;
	mDebugView.type = DebugViewAttachmentType::None;
}
