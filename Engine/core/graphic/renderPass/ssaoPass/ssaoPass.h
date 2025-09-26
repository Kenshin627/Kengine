#pragma once
#include <vector>
#include "../renderPass.h"

enum class BlurType
{
	Uniform,
	Gaussian
};

struct SSAOSpecification
{
	uint	 kernelSize		{ 64				};
	float	 samplerRadius	{ 1.0f				};
	BlurType blurType		{ BlurType::Uniform };
	float	 bias			{ 0.1f				};
};

class Texture2D;
class SSAOPass :public RenderPass
{
public:
	SSAOPass(const SSAOSpecification& spec, Renderer* r, const RenderState& state);
	~SSAOPass() = default;
	void setKernelSize(uint kernelSize);
	uint getKernelSize() const;
	void setSamplerRadius(float radius);
	float getSamplerRadius() const;
	void setBias(float bias);
	float getBias() const;
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
	SSAOSpecification		   mSpec;
	std::vector<glm::vec4>	   mKernelSamplers;
	std::unique_ptr<Texture2D> mNoiseTexture;
};