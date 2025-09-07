#pragma once
#include <vector>
#include "../renderPass.h"

class Texture2D;
class SSAOPass :public RenderPass
{
public:
	SSAOPass(uint kernelSize, uint radius, const RenderState& state);
	~SSAOPass() = default;
	void setKernelSize(uint kernelSize);
	uint getKernelSize() const;
	void setRadius(uint radius);
	uint getRadius() const;
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
private:
	void buildSamplers();
	void buildNoiseTexture();
	float lerp(float a, float b, float c)
	{
		return a + (b - a) * c;
	}
private:
	uint					   mKernelSize;
	uint					   mRadius;
	std::vector<glm::vec3>	   mKernelSamplers;
	std::unique_ptr<Texture2D> mNoiseTexture;
};