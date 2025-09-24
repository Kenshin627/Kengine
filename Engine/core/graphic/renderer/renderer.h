#pragma once
#include <glad/glad.h>
#include <memory>
#include <glm.hpp>
#include <unordered_map>
#include <map>
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
	uint kernelSize;
	float samplerRadius;
	float blurRadius;
	BlurType blurType;
};


struct RenderPipeLine
{
	RenderMode                        renderMode{ RenderMode::DefferedShading};
	bool	                          enableBloom{false};
	uint                              bloomDegree{6};
	bool                              enableParallaxOcclusion{ false };
	float                             parallaxOcclusionScale{0.01f};
	bool                              enableCascadedShadowMap{false};
	CascadeShadowMapPassSpecification csmSpec;
	bool						      enableSSao{false};
	SSAOSpecification				  ssaoSpec;
	PostProcessEffect				  postProcess;
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
	void enableParallexOcclusion(bool enable);
	bool getParallaxOcclusion() const;
	float getPOMScale() const;
	void setPOMScale(float scale);
private:
	void setDefaultRenderPass();
	void addRenderPass(RenderPassKey key, const RenderState& state);
	void renderUI();
private:	
	std::shared_ptr<Scene>										   mCurrentScene;
	std::map<RenderPassKey,RenderPass*>					           mRenderPasses;
	uint														   mWidth;
	uint														   mHeight;
	glm::vec4													   mViewport;
	std::unordered_map<RenderPassKey, std::unique_ptr<RenderPass>> mPassCache;
	RenderPipeLine												   mRenderPipeLine;
};