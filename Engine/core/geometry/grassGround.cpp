#include "grassGround.h"

static std::vector<Vertex> vertices = {
   { {-0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f}, {1, 0, 0} },
   { { 0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, {1.0f, 0.0f}, {1, 0, 0} },
   { { 0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, {1.0f, 1.0f}, {1, 0, 0} },
   { {-0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, {0.0f, 1.0f}, {1, 0, 0} }
};


GrassGround::GrassGround(float width, float height, const glm::vec2& subdivision)
	:mWidth(width),
	mHeight(height),
	subdivision(subdivision)
{
	buildGeometry();
}


GrassGround::~GrassGround()
{
}

void GrassGround::buildGeometry()
{
	//change vertices data depend on width and height
	for (Vertex& vertex : vertices)
	{
		vertex.Position.x = vertex.Position.x * mWidth;
		vertex.Position.z = vertex.Position.z * mHeight;
		vertex.Texcoord.x = vertex.Texcoord.x * mWidth ;
		vertex.Texcoord.y = vertex.Texcoord.y * mHeight ;
	}
	std::unique_ptr<VertexArray> vao = std::make_unique< VertexArray>(4, GL_PATCHES);
	uint vboId = vao->buildVertexBuffer(sizeof(Vertex) * vertices.size(), (void*)vertices.data(), GL_DYNAMIC_STORAGE_BIT);
	
	uint stride = sizeof(Vertex);
	std::initializer_list<AttributeLayout> layouts = {
		{0, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Position), 0},
		{1, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Normal), 0},
		{2, vboId, 0, 0, stride, 2, GL_FLOAT, false, offsetof(Vertex, Texcoord), 0},
		{3, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Tangent),0}
	};
	vao->addAttributes(layouts);
	setVAO(std::move(vao));
	glPatchParameteri(GL_PATCH_VERTICES, 4);
}
