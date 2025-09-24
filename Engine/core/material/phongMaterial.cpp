#include "phongMaterial.h"
#include "graphic/texture/texture2D/texture2D.h"

PhongMaterial::PhongMaterial(const BlinnPhongMaterialSpecification& spec)
	:mDiffuseMap(spec.diffuseMap),
	 mSpecularMap(spec.specularMap),
	 mNormalMap(spec.normalMap),
	 mShinessMap(spec.shinessMap),
	 mHeightMap(spec.heightMap),
	 mAlphaMap(spec.alphaMap),
	 mDiffuseColor(spec.diffuseColor),
	 mSpecularColor(spec.specularColor),
	 mEmissiveColor(spec.emissiveColor),
	 mShiness(spec.shiness)
{
	mHasDiffuseTex  = mDiffuseMap  ? true : false;
	mHasSpecularTex = mSpecularMap ? true : false;
	mHasNormalTex   = mNormalMap   ? true : false;
	mHasShinessTex  = mShinessMap  ? true : false;
	mhasHeighTex	= mHeightMap   ? true : false;
	mHasAlphaTex    = mAlphaMap    ? true : false;
	if (mEmissiveColor.r != 0 || mEmissiveColor.g != 0 || mEmissiveColor.b != 0)
	{
		mIsEmissive = true;
	}
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

	if (mhasHeighTex)
	{
		mHeightMap->bind(4);
		p->setUniform("heightMap", 4);
	}

	if (mHasAlphaTex)
	{
		mAlphaMap->bind(5);
		p->setUniform("alphaMap", 5);
	}

	p->setUniform("materialType", 0);

	//colors
	p->setUniform("diffuseColor", mDiffuseColor);
	p->setUniform("specularColor", mSpecularColor);
	p->setUniform("emissiveColor", mEmissiveColor);
	p->setUniform("shiness", mShiness);

	//bool
	p->setUniform("hasDiffTex", mHasDiffuseTex);
	p->setUniform("hasSpecTex", mHasSpecularTex);
	p->setUniform("hasNormalTex", mHasNormalTex);
	p->setUniform("hasShinessTex", mHasShinessTex);
	p->setUniform("isEmissive", mIsEmissive);
	p->setUniform("hasHeightTex", mhasHeighTex);
	p->setUniform("hasAlphaTex", mHasAlphaTex);
	p->setUniform("hasAlphaTest", mHasAlphaTest);
}

void PhongMaterial::initProgram()
{
	std::initializer_list<ShaderFile> files = {
		{ "core/graphic/shaderSrc/blinnPhong/vs.glsl", ShaderType::Vertex },
		{ "core/graphic/shaderSrc/blinnPhong/fs.glsl",ShaderType::Fragment }
	};
	mProgram->buildFromFiles(files);
}

void PhongMaterial::setDiffuseColor(const glm::vec3& col)
{
	mDiffuseColor = col;
}

void PhongMaterial::setEmissiveColor(const glm::vec3& col)
{
	mEmissiveColor = col;
}

void PhongMaterial::endDraw()
{
	if (mHasDiffuseTex)
	{
		mDiffuseMap->unBind(0);
	}

	if (mHasSpecularTex)
	{
		mSpecularMap->unBind(1);
	}


	if (mHasNormalTex)
	{
		mNormalMap->unBind(2);
	}

	if (mHasShinessTex)
	{
		mShinessMap->unBind(3);
	}

	if (mhasHeighTex)
	{
		mHeightMap->unBind(4);
	}

	if (mHasAlphaTex)
	{
		mAlphaMap->unBind(5);
	}
}

