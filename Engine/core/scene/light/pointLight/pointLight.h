#pragma once
#include <glm.hpp>

struct polintLightStuctAlignment
{
	glm::vec4 position;  // w is padding
	glm::vec4 color;     // w is padding
	glm::vec4 params;    // x: constant, y: linear, z: quadratic, w is padding
};

class PointLight
{
public:
	PointLight(const glm::vec3& pos, const glm::vec3& color, float kc, float kl, float kq);
	~PointLight() = default;
	PointLight(const PointLight& obj) = delete;
	PointLight(PointLight&& obj) = delete;
	PointLight& operator=(const PointLight& obj) = delete;
	void setPosition(const glm::vec3& pos);
	void setPosition(float pos[3]);
	void setPosition(float x, float y, float z);
	const glm::vec3& getPosition() const;
	void setColor(const glm::vec3& color);
	void setColor(float color[3]);
	void setColor(float r, float g, float b);
	const glm::vec3& getColor() const;
	void setConstant(float kc);
	void setLinear(float kl);
	void setQuadratic(float kq);
	float getConstant() const;
	float getLinear() const;
	float getQuadratic() const;
private:
	glm::vec3  mPosition{ 0.0f, 0.0f, 0.0f };
	glm::vec3  mColor{ 1.0f, 1.0f, 1.0f };
	float	   mConstant{ 1.0f };
	float	   mLinear{ 0.09f };
	float	   mQuadratic{ 0.032f };
};
