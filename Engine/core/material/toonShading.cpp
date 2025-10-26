#include "toonShading.h"
#include "graphic/texture/texture2D/texture2D.h"

ToonShading::ToonShading(const ToonShadingSpecification& spec)
{
	mDiffuseMap = spec.diffuseMap;
	mNormalMap = spec.normalMap;
	mDiffuseColor = spec.diffuseColor;
	mHasDiffuseTex = mDiffuseMap ? true : false;
	mHasNormalTex = mNormalMap ? true : false;
	initProgram();
}

void ToonShading::setUniforms(Program* p) const
{
	if (mHasDiffuseTex)
	{
		mDiffuseMap->bind(0);
		p->setUniform("albedoMap", 0);
	}

	if (mHasNormalTex)
	{
		mNormalMap->bind(1);
		p->setUniform("normalMap", 1);
	}
	//colors
	p->setUniform("albedoColor", mDiffuseColor);
	p->setUniform("hasDiffTex", mHasDiffuseTex);
	p->setUniform("hasNormalTex", mHasNormalTex);
}

void ToonShading::initProgram()
{
	std::initializer_list<ShaderFile> files = {
		{ "core/graphic/shaderSrc/toonShading/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/toonShading/fs.glsl",ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);
}
