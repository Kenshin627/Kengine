#pragma once
#include "../renderPass.h"

struct GaussianBlurSpecification
{
	float Scale		{ 1.0f  };
	float Strength	{ 1.0f  };
	uint  Amount	{ 4     };
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
protected:
	virtual bool checkFrameBuffer() override;
private:
	std::vector<std::shared_ptr<FrameBuffer>> mDoubleBuffers;
	GaussianBlurSpecification mSpec;
};