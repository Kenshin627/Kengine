#pragma once
#include "../light.h"

class SpotLight:public Light
{
public:
	SpotLight(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& color, float kc, float ks, float kq, float theta = 17.5f, float phi = 12.5f);
	~SpotLight() = default;
	void setInner(float theta);
	float getInner() const;
	float getCosInner() const;
	void setOutter(float phi);
	float getOutter() const;
	float getCosOutter() const;
private:
	float mInnerCutoff;
	float mOutterCutoff;
	float mInnerTheta;
	float mOutterPhi;
};