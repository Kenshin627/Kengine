#pragma once
#include<glm.hpp>
#include<vector>
#include"../renderObject.h"

enum class LightType
{
	PointLight = 0,
	SpotLight  = 1
};

//std140 alignment
struct GPULightBufferData
{
	glm::vec4 position		 { 0,0,0,0				     };
	glm::vec4 direction		 { 0, -1, 0, 0			     };
	glm::vec4 color			 { 1, 1, 1, 0			     };
	glm::vec4 attentionFactor{ 1.0f, 0.09f, 0.032f, 0.0f };
	float	  outterCutoff	 { 17.5f					 };
	float	  innerCutoff	 { 12.5f					 };
	LightType type			 { LightType::PointLight	 };
	int		  lightCount	 { 0					     };
};

struct GPULightBufferDataWrapper
{
	std::vector<GPULightBufferData> lights;
	glm::vec4 lightCount;
};

class Light:public RenderObject
{
public:
	Light(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& col, float kc, float ks, float kq);
	virtual ~Light() = default;
	void setPosition(const glm::vec3& pos);
	void setPosition(float pos[3]);
	void setPosition(float x, float y, float z);
	const glm::vec3& getPosition() const;

	void setDirection(const glm::vec3& pos);
	void setDirection(float pos[3]);
	void setDirection(float x, float y, float z);
	const glm::vec3& getDirection() const;

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
	LightType getType() const;
	void updateLightBuffer();
protected:
	LightType  mType;
private:
	glm::vec3  mPosition{ 0.0f, 0.0f, 0.0f };
	glm::vec3  mDirection{ 0.0, -1.0, 0.0 };
	glm::vec3  mColor{ 1.0f, 1.0f, 1.0f };
	float	   mConstant{ 1.0f };
	float	   mLinear{ 0.09f };
	float	   mQuadratic{ 0.032f };
};