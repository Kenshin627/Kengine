#include "screenQuad.h"

//SCREEN QUAD
const std::vector<ScreenQuadVertex> vertices = {
   {{ -1.0f, -1.0f },  { 0.0f, 0.0f}}, 
   {{  1.0f, -1.0f },  { 1.0f, 0.0f}}, 
   {{  1.0f,  1.0f },  { 1.0f, 1.0f}}, 
   {{ -1.0f,  1.0f },  { 0.0f, 1.0f}}  
};

const std::vector<uint> indices = {
	0, 1, 2,
	2, 3, 0
};

ScreenQuad::ScreenQuad()
{
	buildGeometry();
}

ScreenQuad::~ScreenQuad()
{
}

void ScreenQuad::buildGeometry()
{
	std::unique_ptr<VertexArray> vao = std::make_unique< VertexArray>(6, GL_TRIANGLES);
	uint vboId = vao->buildVertexBuffer(sizeof(ScreenQuadVertex) * vertices.size(), (void*)vertices.data(), 0);
	vao->buildIndexBuffer(indices.data(), sizeof(uint) * indices.size(), GL_UNSIGNED_INT, 0);
	uint stride = sizeof(ScreenQuadVertex);
	std::initializer_list<AttributeLayout> layouts = {
		{0, vboId, 0, 0, stride, 2, GL_FLOAT, false, offsetof(ScreenQuadVertex, Position), 0},
		{1, vboId, 0, 0, stride, 2, GL_FLOAT, false, offsetof(ScreenQuadVertex, Texcoord), 0}
	};
	vao->addAttributes(layouts);
	setVAO(std::move(vao));
}
