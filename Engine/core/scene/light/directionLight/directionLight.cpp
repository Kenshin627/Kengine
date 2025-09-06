#include "directionLight.h"

DirectionLight::DirectionLight(const glm::vec3& dir, const glm::vec3& color)
	:mDirection(dir),
	 mColor(color)
{
}

void DirectionLight::setDirection(const glm::vec3& dir)
{
	setDirection(dir.x, dir.y, dir.z);
}

void DirectionLight::setDirection(float dir[3])
{

	setDirection(dir[0], dir[1], dir[2]);
}

void DirectionLight::setDirection(float x, float y, float z)
{
	mDirection.x = x;
	mDirection.y = y;
	mDirection.z = z;
}

const glm::vec3& DirectionLight::getDirection() const
{
	return mDirection;
}

void DirectionLight::setColor(const glm::vec3& color)
{
	setColor(color.x, color.y, color.z);
}

void DirectionLight::setColor(float color[3])
{
	setColor(color[0], color[1], color[2]);
}

void DirectionLight::setColor(float r, float g, float b)
{
	mColor.x = r;
	mColor.y = g;
	mColor.z = b;
}

const glm::vec3& DirectionLight::getColor() const
{
	return mColor;
}
