#pragma once
#include "material.h"

struct GrassMaterialSpecification
{
	float bladeWidth{ 0.05 };
	float bladeWidthRandom{ 0.02 };
	float bladeHeight{ 0.5 };
	float bladeHeightRandom{ 0.1 };
	float bladeBendRandom{ 0.2 };
	float bladeForward{ 0.38 };
	float bladeCurve{ 2.0 };
	glm::vec3 bladeTopColor{ 146.0/255.0, 213.0/255.0, 83.0/255.0 };
	glm::vec3 bladeBottomColor{ 16.0/255.0, 96.0/255.0, 18.0/255.0};
};

class GrassMaterial : public Material
{
public:
	GrassMaterial(const GrassMaterialSpecification& spec = GrassMaterialSpecification());
	~GrassMaterial() = default;
	virtual void setUniforms(Program* p) const override;
	virtual void initProgram() override;
private:
	GrassMaterialSpecification mSpec;
};
