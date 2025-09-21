#pragma once
#include "../renderPass.h"

class Scene;
class UniformBuffer;
class CascadeShadowMapPass :public RenderPass
{
public:
	CascadeShadowMapPass(Scene* scene, uint pcfSize, uint cascadedLayer, const RenderState& state);
	~CascadeShadowMapPass();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
	const std::vector<float>& getCascadedFrustumDistanes() const;
	int getCascadedLayerCount() const;
	int getShadowLightIndex() const;
	int getPcfSize() const;
private:
	std::vector<glm::vec4> getFrustumWorldSpaceCorners(float near, float far);
	glm::mat4 getFrustaProjectionViewMatrix();
	void updateLightMatricesBuffer();
private:
	Scene* mScene;
	std::unique_ptr<UniformBuffer> mLightMatricesBuffer;
	std::vector<float>             mCascadeFrustumDistances;
	int							   mCascadedLayerNum;
	uint						   mPcfSize;
	uint						   mCascadedLayer;
};