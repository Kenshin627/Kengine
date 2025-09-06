#include "pointLight.h"

PointLight::PointLight(const glm::vec3& pos, const glm::vec3& color, float kc, float kl, float kq)
	:mPosition(pos),
	 mColor(color),
	 mConstant(kc),
	 mLinear(kl),
	 mQuadratic(kq)
{
}


void PointLight::setPosition(const glm::vec3& pos)
{
	setPosition(pos.x, pos.y, pos.z);
}

void PointLight::setPosition(float pos[3])
{
	setPosition(pos[0], pos[1], pos[2]);
}

void PointLight::setPosition(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;
}

const glm::vec3& PointLight::getPosition() const
{
	return mPosition;
}

void PointLight::setColor(const glm::vec3& color)
{
	setColor(color.x, color.y, color.z);
}

void PointLight::setColor(float color[3])
{
	setColor(color[0], color[1], color[2]);
}

void PointLight::setColor(float r, float g, float b)
{
	mColor.x = r;
	mColor.y = g;
	mColor.z = b;
}

const glm::vec3& PointLight::getColor() const
{
	return mColor;
}

void PointLight::setConstant(float kc)
{
	mConstant = kc;
}

void PointLight::setLinear(float kl)
{
	mLinear = kl;
}

void PointLight::setQuadratic(float kq)
{
	mQuadratic = kq;
}

float PointLight::getConstant() const
{
	return mConstant;
}

float PointLight::getLinear() const
{
	return mLinear;
}

float PointLight::getQuadratic() const
{
	return mQuadratic;
}
