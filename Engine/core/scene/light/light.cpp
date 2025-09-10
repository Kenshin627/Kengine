#include "light.h"

Light::Light(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& color, float kc, float kl, float kq)
	:mPosition(pos),
	 mDirection(glm::normalize(dir)),
	 mColor(color),
	 mConstant(kc),
	 mLinear(kl),
	 mQuadratic(kq),
	 mType(LightType::PointLight)
{
}

void Light::setPosition(const glm::vec3& pos)
{
	setPosition(pos.x, pos.y, pos.z);
}

void Light::setPosition(float pos[3])
{
	setPosition(pos[0], pos[1], pos[2]);
}

void Light::setPosition(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;
}

const glm::vec3& Light::getPosition() const
{
	return mPosition;
}

void Light::setDirection(const glm::vec3& dir)
{
	setDirection(dir.x, dir.y, dir.z);
}

void Light::setDirection(float dir[3])
{
	setDirection(dir[0], dir[1], dir[2]);
}

void Light::setDirection(float x, float y, float z)
{
	glm::vec3 dir = glm::normalize(glm::vec3{ x, y, z });
	mDirection.x = dir.x;
	mDirection.y = dir.y;
	mDirection.z = dir.z;
}

const glm::vec3& Light::getDirection() const
{
	return mDirection;
}

void Light::setColor(const glm::vec3& color)
{
	setColor(color.x, color.y, color.z);
}

void Light::setColor(float color[3])
{
	setColor(color[0], color[1], color[2]);
}

void Light::setColor(float r, float g, float b)
{
	mColor.x = r;
	mColor.y = g;
	mColor.z = b;
}

const glm::vec3& Light::getColor() const
{
	return mColor;
}

void Light::setConstant(float kc)
{
	mConstant = kc;
}

void Light::setLinear(float kl)
{
	mLinear = kl;
}

void Light::setQuadratic(float kq)
{
	mQuadratic = kq;
}

float Light::getConstant() const
{
	return mConstant;
}

float Light::getLinear() const
{
	return mLinear;
}

float Light::getQuadratic() const
{
	return mQuadratic;
}

LightType Light::getType() const
{
	return mType;
}
