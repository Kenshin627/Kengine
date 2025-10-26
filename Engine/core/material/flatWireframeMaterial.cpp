#include "flatWireframeMaterial.h"

FlatWireframeMaterial::FlatWireframeMaterial(const FlatWireframeMaterialSpecification& spec)
{
	mFaceColor = spec.faceColor;
	mLineColor = spec.lineColor;
	mThickness = spec.thickness;
	mSmoothing = spec.smoothing;
	initProgram();
}

void FlatWireframeMaterial::setUniforms(Program* p) const
{
	p->setUniform("faceColor", mFaceColor);
	p->setUniform("lineColor", mLineColor);
	p->setUniform("thickness", mThickness);
	p->setUniform("smoothing", mSmoothing);
}

void FlatWireframeMaterial::initProgram()
{
	std::initializer_list<ShaderFile> files = {
		{ "core/graphic/shaderSrc/flatWireframe/vs.glsl", ShaderType::Vertex   },
		{ "core/graphic/shaderSrc/flatWireframe/gs.glsl", ShaderType::Geometry },
		{ "core/graphic/shaderSrc/flatWireframe/fs.glsl",ShaderType::Fragment  }
	};
	mProgram->buildFromFiles(files);
}

void FlatWireframeMaterial::setThickness(float thickness)
{
	mThickness = thickness;
}

void FlatWireframeMaterial::setSmoothing(float smoothing)
{
	mSmoothing = smoothing;
}

void FlatWireframeMaterial::setFaceColor(const glm::vec3& faceColor)
{
	mFaceColor = faceColor;
}

void FlatWireframeMaterial::setLineColor(const glm::vec3& lineColor)
{
	mLineColor = lineColor;
}
