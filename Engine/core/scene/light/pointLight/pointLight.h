#pragma once
#include <glm.hpp>
#include "../light.h"

class PointLight:public Light
{
public:
	PointLight(const glm::vec3& pos, const glm::vec3& color, float kc, float kl, float kq);
	~PointLight() = default;
	PointLight(const PointLight& obj) = delete;
	PointLight(PointLight&& obj) = delete;
	PointLight& operator=(const PointLight& obj) = delete;	
};
