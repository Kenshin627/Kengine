#include "pbrMaterial.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "graphic/texture/textureSystem.h"

PBRMaterial::PBRMaterial(const PBRMaterialSpecification& spec)
	:Material(),
	mAlbedoColor(spec.albedoColor),
	mMetallic(spec.metallic),
	mRoughness(spec.roughness)
{
	TextureSystem& ts = TextureSystem::getInstance();
	if (spec.albedoMapPath)
	{
		mAlbedoMap = ts.getTexture(spec.albedoMapPath);
		mHasAlbedoTex = true;
	}

	if (spec.metallicMapPath)
	{
		mMetallicMap = ts.getTexture(spec.metallicMapPath);
		mHasMetallicTex = true;
	}

	if (spec.roughnessMapPath)
	{
		mRoughnessMap = ts.getTexture(spec.roughnessMapPath);
		mHasRoughnessTex = true;
	}

	if (spec.normalMapPath)
	{
		mNormalMap = ts.getTexture(spec.normalMapPath);
		mHasNormalTex = true;
	}

	if (spec.heightMapPath)
	{
		mHeightMap = ts.getTexture(spec.heightMapPath);
		mhasHeightTex = true;
	}
}

PBRMaterial::~PBRMaterial()
{
}

void PBRMaterial::setUniforms(Program* p) const
{
	if (mHasAlbedoTex)
	{
		mAlbedoMap->bind(0);
		p->setUniform("diffuseMap", 0);
	}

	if (mHasMetallicTex)
	{
		mMetallicMap->bind(1);
		p->setUniform("specularMap", 1);
	}


	if (mHasNormalTex)
	{
		mNormalMap->bind(2);
		p->setUniform("normalMap", 2);
	}

	if (mHasRoughnessTex)
	{
		mRoughnessMap->bind(3);
		p->setUniform("shinessMap", 3);
	}

	if (mhasHeightTex)
	{
		mHeightMap->bind(4);
		p->setUniform("heightMap", 4);
	}

	p->setUniform("materialType", 1);

	//colors
	p->setUniform("albedoColor", mAlbedoColor);
	p->setUniform("metallic", mMetallic);
	p->setUniform("roughness", mRoughness);

	//bool
	p->setUniform("hasAlbedoTex", mHasAlbedoTex);
	p->setUniform("hasMetallicTex", mHasMetallicTex);
	p->setUniform("hasNormalTex", mHasNormalTex);
	p->setUniform("hasRoughnessTex", mHasRoughnessTex);
	p->setUniform("hasHeightTex", mhasHeightTex);
}
