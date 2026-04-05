#pragma once
#include <glad/glad.h>
#include <memory>
#include <glm.hpp>
#include <unordered_map>
#include <list>
#include "graphic/renderPass/cascadeShadowMapPass/cascadeShadowMapPass.h"

class Scene;
class RenderPass;
enum class RenderPassKey;

enum class RenderMode
{
	DefferedShading,
	ForwardShading
};

enum class PostProcessEffect
{
	GrayScale,
	RGBSplit
};

enum class RenderStrategy
{
	Forward,
	Deferred
};

struct RenderPipeLine
{
	RenderMode                        renderMode			  { RenderMode::DefferedShading};
	bool	                          enableBloom			  { false					  };
	bool							  debugBloom			  { false					  };
	bool						      enableSSao			  { false					  };
	bool							  debugSSAO				  { false					  };
	bool                              enableParallaxOcclusion { false					  };
	float                             parallaxOcclusionScale  { 0.01f					  };
	bool                              enableCascadedShadowMap { false					  };
	bool							  debugCSM	              { false					  };
	PostProcessEffect				  postProcess;
	bool							  enableCannyEdgeDetection{ false					  };
	bool							  enableSelection		  { false					  };
	bool							  debugSelection		  { false					  };
	bool							  enablePCSS			  { false					  };
	bool							  debugPCSS				  { false					  };
	bool							  enableFXAA			  { false					  };
	bool							  debugFXAA				  { false					  };
};

struct RenderKeyPass
{
	RenderPassKey				  key;
	std::shared_ptr<RenderPass>   pass;
};

enum class DebugViewAttachmentType
{
	Color = 0,
	Depth,
	None
};

struct DebugView
{
	Texture* texture{ nullptr };
	RenderPassKey key{ RenderPassKey::BLOOM};
	bool operator ==(const DebugView& v)
	{
		return key == v.key;
	}
};

class Renderer
{
public:
	Renderer(uint width, uint height);
	~Renderer();
	void render(double deltaTime);
	void setCurrentScene(std::shared_ptr<Scene> scene) { mCurrentScene = scene; }
	Scene* getCurrentScene();
	void onWindowSizeChanged(uint width, uint height);
	uint getLastFrameBufferTexture() const;
	FrameBuffer* getFrameBuffer(RenderPassKey key) const;
	RenderPass* getRenderPass(RenderPassKey key) const;
	RenderStrategy getRenderStrategy() const { return mRenderStrategy; }
public:
	//POM
	void enableParallexOcclusion(bool enable);
	bool getParallaxOcclusion() const;
	float getPOMScale() const;
	void setPOMScale(float scale);

	//BLOOM
	void setEnableBloom(bool enable);
	bool getEnableBloom() const;
	uint getBlurRadius() const;
	void setBlurRadius(uint blur);

	float getBloomBlurStrength();
	void setBloomBlurStrength(float s);
	float getGaussianSigma() const;
	void setGaussianSigma(float s);
	void setThresholdMin(float t);
	float getThresholdMin() const;
	void setThresholdMax(float t);
	float getThresholdMax() const;

	//SSAO
	//TODO:BLURTYPE
	void enableSSAO(bool enable);
	bool getEnableSSAO() const;
	uint getSSAOKernelSize() const;
	void setSSAOKernelSize(uint kernelSize);
	float getSSAOBias() const;
	void setSSAOBias(float bias);
	uint getSSAOBlurRadius() const;
	void setSSAOBlurRadius(float blurRadius);
	uint getSSAOSamplerRadius() const;
	void setSSAOSamplerRadius(float samplerRadius);
	uint getSSAOBlur() const;
	void setSSAOBlur(uint blur);
	float getSSAOBlurScale();
	void setSSAOBlurScale(float s);
	float getSSAOBlurStrength();
	void setSSAOBlurStrength(float s);
	float getSSAOSigma() const;
	void setSSAOSigma(float s);


	const std::vector<DebugView>& getDebugView() const;

	//CSM
	void enableCSM(bool enable);
	bool getEnableCSM() const;

	void enablePCSS(bool enable);
	bool getEnablePCSS(bool enable) const;

	//canny edge detection
	void enableCannyEdgeDetection(bool enable);
	void setLowEdgeTrheshold(float low);
	float getLowEdgeThreshold() const;
	void setHighEdgeTrheshold(float high);
	float getHighEdgeThreshold() const;

	//selection
	void enableSelection(bool enable);

	//FXAA
	void enableFXAA(bool enable);

	//TODO: edge color edge width blurAmount
	RenderPass* addRenderPass(RenderPassKey key, const RenderState& state, RenderPass* where);
private:
	void setDefaultRenderPass();
	void removePass(RenderPassKey key);
	void renderUI();
	void popDebugView(RenderPassKey key);
private:
	std::shared_ptr<Scene>										   mCurrentScene;
	std::list<RenderPass*>										   mCurrentRenderPassGroup;
	uint														   mWidth;
	uint														   mHeight;
	glm::vec4													   mViewport;
	std::unordered_map<RenderPassKey, RenderKeyPass>			   mPassCache;
	RenderPipeLine												   mRenderPipeLine;
	std::vector<DebugView>										   mDebugViews;
	RenderStrategy												   mRenderStrategy{ RenderStrategy::Deferred };
};