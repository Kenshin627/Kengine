#pragma once
#include "material.h"

struct FlatWireframeMaterialSpecification
{
	glm::vec3 faceColor { 1.0f, 1.0f, 1.0f };
	glm::vec3 lineColor { 0.0f, 0.0f, 0.0f };
	float thickness		{ 1.2f			   };
	float smoothing		{ 1.0f			   };
};

class FlatWireframeMaterial :public Material
{
public:
	FlatWireframeMaterial(const FlatWireframeMaterialSpecification& spec = FlatWireframeMaterialSpecification());
	virtual ~FlatWireframeMaterial() = default;
	FlatWireframeMaterial(const FlatWireframeMaterial& obj) = delete;
	FlatWireframeMaterial(FlatWireframeMaterial&& obj) = delete;
	FlatWireframeMaterial& operator=(const FlatWireframeMaterial& obj) = delete;
	virtual void setUniforms(Program* p) const override;
	virtual void initProgram() override;
	void setThickness(float thickness);
	void setSmoothing(float smoothing);
	void setFaceColor(const glm::vec3& faceColor);
	void setLineColor(const glm::vec3& lineColor);
private:
	glm::vec3  mFaceColor{ 1.0f, 1.0f, 1.0f };
	glm::vec3  mLineColor{ 0.2f, 0.2f, 0.2f };
	float      mThickness{ 1.0f };
	float      mSmoothing{ 1.0f };
};