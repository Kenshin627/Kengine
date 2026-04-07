#pragma once
#include <memory>
#include <glm.hpp>
#include "../graphic/gpuBuffer/vertexArray.h"

#define MAX_BONE_INFLUENCE 4
static const float PI = 3.1415926535897932384626433832795;

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 Texcoord;
	glm::vec3 Tangent;
	int		  BoneIds[MAX_BONE_INFLUENCE];
	float	  Weights[MAX_BONE_INFLUENCE];
};

struct ScreenQuadVertex
{
	glm::vec2 Position;
	glm::vec2 Texcoord;
};

class Geometry
{
public:
	Geometry();
	virtual ~Geometry() = default;
	virtual void buildGeometry() {};
	Geometry(const Geometry& obj) = delete;
	Geometry(Geometry&& obj) = delete;
	Geometry& operator=(const Geometry& obj) = delete;
	void beginDraw() const;
	void endDraw();
	void draw() const;
	void setVAO(std::unique_ptr<VertexArray> vao) { mVAO = std::move(vao); }
private:
	std::unique_ptr<VertexArray> mVAO;
};