#pragma once
#include "../light.h"

class SpotLight:public Light
{
public:
	SpotLight(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& color, float kc, float ks, float kq, float inner = 17.5f, float outter = 12.5f);
	~SpotLight() = default;
	void setInner(float inner);
	float getInner() const;
	float getCosInner() const;
	void setOutter(float outter);
	float getOutter() const;
	float getCosOutter() const;
private:
	float mInnerCutoff;
	float mOutterCutoff;
	float mOutter;
	float mInner;
};