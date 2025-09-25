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

enum class BlurType
{
	Uniform,
	Gaussian
};

struct SSAOSpecification
{
	uint	 kernelSize;
	float	 samplerRadius;
	float	 blurRadius;
	BlurType blurType;
};


struct RenderPipeLine
{
	RenderMode                        renderMode{ RenderMode::DefferedShading};
	bool	                          enableBloom{false};
	bool                              enableParallaxOcclusion{ false };
	float                             parallaxOcclusionScale{0.01f};
	bool                              enableCascadedShadowMap{false};
	CascadeShadowMapPassSpecification csmSpec;
	bool						      enableSSao{false};
	SSAOSpecification				  ssaoSpec;
	PostProcessEffect				  postProcess;
};

struct RenderKeyPass
{
	RenderPassKey				  key;
	std::shared_ptr<RenderPass>   pass;
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
	int getPassBufferTexture(RenderPassKey key);
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
	uint getSSAOBlurRadius() const;
	void setSSAOBlurRadius(float blurRadius);
	uint getSSAOSamplerRadius() const;
	void setSSAOSamplerRadius(float samplerRadius);
private:
	void setDefaultRenderPass();
	RenderPass* addRenderPass(RenderPassKey key, const RenderState& state, RenderPass* where);
	void removePass(RenderPassKey key);
	void renderUI();
private:	
	std::shared_ptr<Scene>										   mCurrentScene;
	std::list<RenderPass*>										   mCurrentRenderPassGroup;
	uint														   mWidth;
	uint														   mHeight;
	glm::vec4													   mViewport;
	std::unordered_map<RenderPassKey, RenderKeyPass>			   mPassCache;
	RenderPipeLine												   mRenderPipeLine;
};