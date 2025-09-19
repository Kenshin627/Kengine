#include "spotLight.h"

SpotLight::SpotLight(const std::string& name, const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& color, float kc, float ks, float kq, float outter, float inner)
	:Light(name, pos, dir, color, kc, ks, kq),
	mOutterCutoff(glm::cos(glm::radians(outter))),
	mInnerCutoff(glm::cos(glm::radians(inner))),
	mOutter(outter),
	mInner(inner)
{
	mType = LightType::SpotLight;
}

void SpotLight::setInner(float inner)
{
	if (inner > mOutter)
	{
		inner = mOutter - 0.01;
	}
	mInnerCutoff = glm::cos(glm::radians(inner));
	mInner = inner;
	updateLightBuffer();
}

float SpotLight::getOutter() const
{
	return mOutter;
}

float SpotLight::getCosInner() const
{
	return mInnerCutoff;
}

void SpotLight::setOutter(float outter)
{
	//clamp
	if (outter <= mInner)
	{
		outter = mInner + 0.01;
	}
	mOutterCutoff = glm::cos(glm::radians(outter));
	mOutter = outter;
	updateLightBuffer();
}

float SpotLight::getInner() const
{
	return mInner;
}

float SpotLight::getCosOutter() const
{
	return mOutterCutoff;
}
