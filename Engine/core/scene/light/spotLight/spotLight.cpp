#include "spotLight.h"

SpotLight::SpotLight(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& color, float kc, float ks, float kq, float theta, float phi)
	:Light(pos, dir, color, kc, ks, kq),
	mInnerCutoff(glm::cos(glm::radians(phi))),
	mOutterCutoff(glm::cos(glm::radians(theta)))
{
	mType = LightType::SpotLight;
}

void SpotLight::setInner(float theta)
{
	mInnerCutoff = glm::cos(glm::radians(theta));
}

float SpotLight::getInner() const
{
	return mInnerCutoff;
}

void SpotLight::setOutter(float phi)
{
	mOutterCutoff = glm::cos(glm::radians(phi));
}

float SpotLight::getOutter() const
{
	return mOutterCutoff;
}
