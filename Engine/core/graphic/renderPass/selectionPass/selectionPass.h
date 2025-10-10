#pragma once
#include "graphic/renderPass/renderPass.h"

class SelectionPass : public RenderPass
{
public:
	SelectionPass(Renderer* s, const RenderState& state);
	~SelectionPass();
	virtual void beginPass() override;
	virtual void runPass(Scene* scene) override;
	glm::vec3& getColor(uint index) const;
	uint getObjectIndex(const glm::vec2& screenPos) const;
private:

};