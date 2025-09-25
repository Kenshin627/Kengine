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

struct RenderPipeLine
{
	RenderMode                        renderMode			 { RenderMode::DefferedShading};
	bool	                          enableBloom			 { false					  };
	bool							  debugBloom			 { false					  };
	bool						      enableSSao			 { false					  };
	bool							  debugSSAO				 { false					  };
	bool                              enableParallaxOcclusion{ false					  };
	float                             parallaxOcclusionScale { 0.01f					  };
	bool                              enableCascadedShadowMap{ false					  };
	bool							  debugCSM	             { false					  };
	PostProcessEffect				  postProcess;
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
	FrameBuffer* fbo{ nullptr };
	int colorAttachmentIndex{ -1 };
	DebugViewAttachmentType type{ DebugViewAttachmentType::None };
};

class Renderer
{
public:
	Renderer(uint width, uint height);
	~Renderer();
	void render();
	void setCurrentScene(std::shared_ptr<Scene> scene) { mCurrentScene = scene; }
	Scene* getCurrentScene();
	void onWindowSizeChanged(uint width, uint height);
	uint getLastFrameBufferTexture() const;
	FrameBuffer* getFrameBuffer(RenderPassKey key) const;
	RenderPass* getRenderPass(RenderPassKey key) const;
public:
	//POM
	void enableParallexOcclusion(bool enable);
	bool getParallaxOcclusion() const;
	float getPOMScale() const;
	void setPOMScale(float scale);

	//BLOOM
	void setEnableBloom(bool enable);
	bool getEnableBloom() const;
	uint getBloomBlur() const;
	void setBloomBlur(uint blur);
	float getBloomBlurScale();
	void setBloomBlurScale(float s);
	float getBloomBlurStrength();
	void setBloomBlurStrength(float s);

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
	const DebugView& getDebugView() const;

	//CSM
	void enableCSM(bool enable);
	bool getEnableCSM() const;
private:
	void setDefaultRenderPass();
	RenderPass* addRenderPass(RenderPassKey key, const RenderState& state, RenderPass* where);
	void removePass(RenderPassKey key);
	void renderUI();
	void resetDebugView();
private:
	std::shared_ptr<Scene>										   mCurrentScene;
	std::list<RenderPass*>										   mCurrentRenderPassGroup;
	uint														   mWidth;
	uint														   mHeight;
	glm::vec4													   mViewport;
	std::unordered_map<RenderPassKey, RenderKeyPass>			   mPassCache;
	RenderPipeLine												   mRenderPipeLine;
	DebugView													   mDebugView;
};