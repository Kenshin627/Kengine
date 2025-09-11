#include "phongMaterial.h"
#include "graphic/texture/texture2D/texture2D.h"

PhongMaterial::PhongMaterial(const BlinnPhongMaterialSpecification& spec)
	:mDiffuseMap(spec.diffuseMap),
	 mSpecularMap(spec.specularMap),
	 mNormalMap(spec.normalMap),
	 mShinessMap(spec.shinessMap),
	 mDiffuseColor(spec.diffuseColor),
	 mSpecularColor(spec.specularColor),
	 mShiness(spec.shiness)
{
	mHasDiffuseTex  = mDiffuseMap  ? true : false;
	mHasSpecularTex = mSpecularMap ? true : false;
	mHasNormalTex   = mNormalMap   ? true : false;
	mHasShinessTex  = mShinessMap  ? true : false;
	initProgram();
}

void PhongMaterial::setUniforms(Program* p) const
{
	if (mHasDiffuseTex)
	{
		mDiffuseMap->bind(0);
		p->setUniform("diffuseMap", 0);
	}

	if (mHasSpecularTex)
	{
		mSpecularMap->bind(1);
		p->setUniform("specularMap", 1);
	}


	if (mHasNormalTex)
	{
		mNormalMap->bind(2);
		p->setUniform("normalMap", 2);
	}

	if (mHasShinessTex)
	{
		mShinessMap->bind(3);
		p->setUniform("shinessMap", 3);
	}

	//colors
	p->setUniform("diffuseColor", mDiffuseColor);
	p->setUniform("specularColor", mSpecularColor);
	p->setUniform("shiness", mShiness);

	//bool
	p->setUniform("hasDiffTex", mHasDiffuseTex);
	p->setUniform("hasSpecTex", mHasSpecularTex);
	p->setUniform("hasNormalTex", mHasNormalTex);
	p->setUniform("hasShinessTex", mHasShinessTex);
}

void PhongMaterial::initProgram()
{
	std::initializer_list<ShaderFile> files = {
		{ "core/graphic/shaderSrc/blinnPhong/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/blinnPhong/fs.glsl",ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);
}
