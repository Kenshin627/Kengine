#pragma once
#include "../renderPass.h"

constexpr int CascadeLayers = 5;

class Scene;
class UniformBuffer;
class CascadeShadowMapPass :public RenderPass
{
public:
	CascadeShadowMapPass(Scene* scene, const RenderState& state);
	~CascadeShadowMapPass();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
private:
	std::vector<glm::vec4> getFrustumWorldSpaceCorners(float near, float far);
	glm::mat4 getFrustaProjectionViewMatrix();
	void updateLightMatricesBuffer();
private:
	Scene*						   mScene;
	std::unique_ptr<UniformBuffer> mLightMatricesBuffer;
	float		                   cascadeFrustumDistances[4];
};