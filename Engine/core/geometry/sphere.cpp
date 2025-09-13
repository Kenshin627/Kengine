#include <glm.hpp>
#include <ext/scalar_constants.hpp>
#include "sphere.h"
#include "core.h"

Sphere::Sphere(float r, float segment, float ring)
    :Geometry(),
     mRadius(r),
     mSegment(segment),
     mRing(ring)
{
    buildGeometry();
}

Sphere::~Sphere()
{
}

void Sphere::buildGeometry()
{
    if (mSegment < 3)
    {
        KS_CORE_ERROR("segments must be at least 3");
    }
    if (mRing < 2)
    {
        KS_CORE_ERROR("rings must be at least 2");
    };
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float sectorStep = 2 * glm::pi<float>() / mSegment; //sector
    float stackStep = glm::pi<float>() / mRing; //stack

    // 生成顶点
    for (int i = 0; i <= mRing; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * stackStep;
        float xy = mRadius * cosf(stackAngle);
        float z = mRadius * sinf(stackAngle);

        for (int j = 0; j <= mSegment; ++j) {
            float sectorAngle = j * sectorStep;

            //position
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            //normal
            float length = sqrtf(x * x + y * y + z * z);
            float nx = x / length;
            float ny = y / length;
            float nz = z / length;

            //uv
            float u = (float)j / mSegment;
            float v = (float)i / mRing;

            //tangent
             // 计算切线 (沿经度方向)
            glm::vec3 tangent;
            tangent.x = -sinf(sectorAngle);
            tangent.y = cosf(sectorAngle);
            tangent.z = 0.0f;
            tangent = normalize(tangent);

            
            // 添加顶点
            vertices.push_back({ { x, y, z }, { nx, ny, nz }, { u, v }, {tangent.x, tangent.y, tangent.z} });
        }
    }

    // 生成索引
    for (int i = 0; i < mRing; ++i) {
        int k1 = i * (mSegment + 1);
        int k2 = k1 + mSegment + 1;

        for (int j = 0; j < mSegment; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (mRing - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

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