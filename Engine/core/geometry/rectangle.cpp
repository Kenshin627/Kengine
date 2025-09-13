#include "rectangle.h"

std::vector<Vertex> vertices = {
   { {-0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f}, {1, 0, 0} },
   { { 0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, {1.0f, 0.0f}, {1, 0, 0} },
   { { 0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, {1.0f, 1.0f}, {1, 0, 0} },
   { {-0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, {0.0f, 1.0f}, {1, 0, 0} }
};

const std::vector<uint> indices = {
    0, 1, 2, 
    2, 3, 0  
};

Rectangle::Rectangle(float width, float height)
	:mWidth(width),
	 mHeight(height)
{
	buildGeometry();
}


Rectangle::~Rectangle()
{
}

void Rectangle::buildGeometry()
{
	//change vertices data depend on width and height
	for (Vertex& vertex : vertices)
	{
		vertex.Position.x = vertex.Position.x * mWidth;
		vertex.Position.z = vertex.Position.z * mHeight;
		//repeat texture
		vertex.Texcoord.x = vertex.Texcoord.x * mWidth;
		vertex.Texcoord.y = vertex.Texcoord.y * mHeight;
	}
	std::unique_ptr<VertexArray> vao = std::make_unique< VertexArray>(6, GL_TRIANGLES);
	uint vboId = vao->buildVertexBuffer(sizeof(Vertex) * vertices.size(), (void*)vertices.data(), GL_DYNAMIC_STORAGE_BIT);
	vao->buildIndexBuffer(indices.data(), sizeof(uint) * indices.size(), GL_UNSIGNED_INT, GL_DYNAMIC_STORAGE_BIT);
	uint stride = sizeof(Vertex);
	std::initializer_list<AttributeLayout> layouts = {
		{0, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Position), 0},
		{1, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Normal), 0},
		{2, vboId, 0, 0, stride, 2, GL_FLOAT, false, offsetof(Vertex, Texcoord), 0},
		{3, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, tangent),0}
	};
	vao->addAttributes(layouts);
	setVAO(std::move(vao));
}
