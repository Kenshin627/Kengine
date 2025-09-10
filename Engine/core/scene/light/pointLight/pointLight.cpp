#include "pointLight.h"

PointLight::PointLight(const glm::vec3& pos, const glm::vec3& color, float kc, float kl, float kq)
	:Light(pos, { 0, 0, 0 }, color, kc, kl, kq) 
{
	mType = LightType::PointLight;
}
