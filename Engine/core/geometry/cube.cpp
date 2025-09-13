#include "cube.h"
#include <vector>

const std::vector<Vertex> vertices = {
    //前面 (Z=0.5)
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1, 0, 0}},
    {{0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1, 0, 0}},
    {{0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1, 0, 0}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1, 0, 0}},

    //后面 (Z=-0.5)
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {-1, 0, 0}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {-1, 0, 0}},
    {{0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {-1, 0, 0}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {-1, 0, 0}},

    //右面 (X=0.5)
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 1}},
    {{0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0, 0, 1}},
    {{0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0, 0, 1}},
    {{0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0, 0, 1}},

    //左面 (X=-0.5)
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0, 0, -1}},
    {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, -1}},
    {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0, 0, -1}},
    {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0, 0, -1}},

    //上面 (Y=0.5)
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1, 0, 0 }},
    {{0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1, 0, 0}},
    {{0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1, 0, 0}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1, 0, 0}},

    //下面 (Y=-0.5)
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1, 0, 0}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1, 0, 0}},
    {{0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1, 0, 0}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1, 0, 0}}
};

// 索引数据（每个面由2个三角形组成，共6个面×6个索引=36个索引）
const std::vector<uint> indices = {
    // 前面
    0, 1, 2,
    2, 3, 0,

    // 后面
    4, 5, 6,
    6, 7, 4,

    // 右面
    8, 9, 10,
    10, 11, 8,

    // 左面
    12, 13, 14,
    14, 15, 12,

    // 上面
    16, 17, 18,
    18, 19, 16,

    // 下面
    20, 21, 22,
    22, 23, 20
};

Cube::Cube(float width, float length, float height)
	:mWidth(width),
	mLength(length),
	mHeight(height)
{
    buildGeometry();
}

Cube::~Cube()
{
}

void Cube::buildGeometry()
{
	std::unique_ptr<VertexArray> vao = std::make_unique< VertexArray>(indices.size(), GL_TRIANGLES);
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
