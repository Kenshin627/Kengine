#include "../renderPass.h"

class PreDepthPass :public RenderPass
{
public:
	PreDepthPass(const RenderState& state);
	~PreDepthPass() = default;
	virtual void runPass(Scene* scene) override;
private:
};