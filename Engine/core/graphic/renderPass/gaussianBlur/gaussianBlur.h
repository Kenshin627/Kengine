#pragma once
#include "../renderPass.h"

constexpr uint MAX_HALF_KERNEL_SIZE = 21;

struct GaussianBlurSpecification
{
	float Scale		{ 1.0f  };
	float Strength	{ 1.0f  };
	uint  Amount	{ 2     };
	uint  kernelSize;
	float Sigma	    { 1.2f  };
};
class GaussianBlur :public RenderPass
{
public:
	GaussianBlur(const GaussianBlurSpecification& spec, Renderer* r, const RenderState& state);
	~GaussianBlur();
	virtual void beginPass();
	virtual void runPass(Scene* scene) override;
	virtual void endPass() override;
	FrameBuffer* getOutputFrameBuffer();
	virtual void resize(uint width, uint height) override;
	void setGaussianBlurAmount(uint amount);
	uint getGussianBlurAmount() const;
	void setGaussianBlurScale(float s);
	float getGussianBlurScale() const;
	void setGaussianBlurStrength(float strength);
	float getGussianBlurStrength() const;
	void setSigma(float s);
	float getSigma() const;
protected:
	virtual bool checkFrameBuffer() override;
private:
	void generateKernel();
	void computeKernelSize();
private:
	std::vector<std::shared_ptr<FrameBuffer>> mDoubleBuffers;
	GaussianBlurSpecification				  mSpec;
	std::vector<float>						  mKernels;
};