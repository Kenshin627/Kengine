#pragma once
#include <glm.hpp>

class DirectionLight
{
public:
	DirectionLight(const glm::vec3& dir, const glm::vec3& color);
	~DirectionLight() = default;
	DirectionLight(const DirectionLight& obj) = delete;
	DirectionLight(DirectionLight&& obj) = delete;
	DirectionLight& operator=(const DirectionLight& obj) = delete;
	void setDirection(const glm::vec3& dir);
	void setDirection(float dir[3]);
	void setDirection(float x, float y, float z);
	const glm::vec3& getDirection() const;
	void setColor(const glm::vec3& color);
	void setColor(float color[3]);
	void setColor(float r, float g, float b);
	const glm::vec3& getColor() const;
private:
	glm::vec3 mDirection  { -0.2f, -1.0f, -0.3f };
	glm::vec3 mColor	  { 1.0f,   1.0f,  1.0f };
};