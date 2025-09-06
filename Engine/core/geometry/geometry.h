#pragma once
#include <memory>
#include <glm.hpp>
#include "../graphic/gpuBuffer/vertexArray.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
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
	void draw() const;
protected:
	void setVAO(std::unique_ptr<VertexArray> vao) { mVAO = std::move(vao); }
private:
	std::unique_ptr<VertexArray> mVAO;
};