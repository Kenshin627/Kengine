#include "../renderPass.h"

class PreDepthPass :public RenderPass
{
public:
	PreDepthPass(Renderer* r, const RenderState& state);
	~PreDepthPass() = default;
	virtual void runPass(Scene* scene) override;
private:
};