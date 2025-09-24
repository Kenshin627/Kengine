#pragma once
#include "../renderPass.h"

class Scene;
class UniformBuffer;
enum class FrustumSplitMethod
{
	Uniform = 0,
	Log,
	Pratical    //blend uniform with log using splitLambda
};

static const char* splitMethods[] = { "uniform", "log", "Pratical"};


struct CascadeShadowMapPassSpecification
{
	uint pcfSize;
	uint cascadedLayer;
	float splitLambda;
	FrustumSplitMethod splitMethod;
};

class CascadeShadowMapPass :public RenderPass
{
public:
	CascadeShadowMapPass(const CascadeShadowMapPassSpecification& spec, Renderer* r, const RenderState& state);
	~CascadeShadowMapPass();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
	const std::vector<float>& getCascadedFrustumDistanes() const;
	int getCascadedLayerCount() const;
	int getShadowLightIndex() const;
	int getPcfSize() const;
	void selectSplitMethod(FrustumSplitMethod method);
	void setSplitLambda(float lambda);
	void setPcfSize(int size);
	void updateLightMatricesBuffer();
	virtual void renderUI() override;
	void setDisplayCacadedColor(bool show);
	bool getDisplayCacadedColor() const { return mDisplayCacadedColor; };
	void setEnablePCF(bool enable);
	bool getEnablePCF() const { return mEnablePcf; }
	void cascadedSplit();
private:
	void uniformSplit();
	void logarithmicSplit();
	void practicalSplit();
	std::vector<glm::vec4> getFrustumWorldSpaceCorners(float near, float far);
	glm::mat4 getFrustaProjectionViewMatrix();
private:
	std::unique_ptr<UniformBuffer> mLightMatricesBuffer;
	std::vector<float>             mCascadedFrustumSplit;
	uint						   mPcfSize		  { 2	 };
	uint						   mCascadedLayer { 4	 };
	float						   mSplitLambda   { 0.95 };
	FrustumSplitMethod			   mSplitMethod;
	bool						   mDisplayCacadedColor{ false };
	bool						   mEnablePcf{ true };
};