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

    //for (uint ring = 0; ring <= mRing; ring++) 
    //{
    //    // γ�ȽǦȣ��ӱ���(��/2)���ϼ�(-��/2)
    //    float theta = glm::pi<float>() * 0.5f - ring * glm::pi<float>() / mRing;
    //    float sinTheta = std::sin(theta);
    //    float cosTheta = std::cos(theta);

    //    for (uint seg = 0; seg <= mSegment; ++seg) 
    //    {
    //        // ���ȽǦգ���0��2��
    //        float phi = seg * 2.0f * glm::pi<float>() / mSegment;
    //        float sinPhi = std::sin(phi);
    //        float cosPhi = std::cos(phi);

    //        // ���㶥��λ�ã�������ת�ѿ������꣩
    //        glm::vec3 position(
    //            mRadius * sinTheta * cosPhi,  // X
    //            mRadius * sinTheta * sinPhi,  // Y
    //            mRadius * cosTheta            // Z
    //        );

    //        // �����������巨������λ��������ͬ����һ����
    //        glm::vec3 normal = glm::normalize(position);

    //        // �������꣨U: ���ȷ���V: γ�ȷ���
    //        glm::vec2 texCoord(
    //            1.0f - (seg / (float)mSegment),  // U���꣨��ת�Ա����������ҵߵ���
    //            1.0f - (ring / (float)mRing)     // V����
    //        );

    //        vertices.push_back({ position, normal, texCoord });
    //    }
    //}

    //// �����������ݣ�ÿ���ı��β��Ϊ���������Σ�
    //for (unsigned int ring = 0; ring < mRing; ++ring) 
    //{
    //    for (unsigned int seg = 0; seg < mSegment; ++seg) 
    //    {
    //        // ��ǰ�����ĸ���������
    //        unsigned int topLeft = ring * (mSegment + 1) + seg;
    //        unsigned int topRight = topLeft + 1;
    //        unsigned int bottomLeft = (ring + 1) * (mSegment + 1) + seg;
    //        unsigned int bottomRight = bottomLeft + 1;

    //        // ��һ�������Σ����� -> ���� -> ����
    //        indices.push_back(topLeft);
    //        indices.push_back(bottomLeft);
    //        indices.push_back(bottomRight);

    //        // �ڶ��������Σ����� -> ���� -> ����
    //        indices.push_back(topLeft);
    //        indices.push_back(bottomRight);
    //        indices.push_back(topRight);
    //    }
    //}

    float sectorStep = 2 * glm::pi<float>() / mSegment; //sector
    float stackStep = glm::pi<float>() / mRing; //stack

    // ���ɶ���
    for (int i = 0; i <= mRing; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * stackStep;
        float xy = mRadius * cosf(stackAngle);
        float z = mRadius * sinf(stackAngle);

        for (int j = 0; j <= mSegment; ++j) {
            float sectorAngle = j * sectorStep;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            // ���㷨��(��һ����λ������)
            float length = sqrtf(x * x + y * y + z * z);
            float nx = x / length;
            float ny = y / length;
            float nz = z / length;

            // ����UV����
            float u = (float)j / mSegment;
            float v = (float)i / mRing;

            // ��Ӷ���
            vertices.push_back({{ x, y, z }, { nx, ny, nz }, { u, v }});
        }
    }

    // ��������
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
        {1, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Normal),   0},
        {2, vboId, 0, 0, stride, 2, GL_FLOAT, false, offsetof(Vertex, Texcoord), 0}
    };
    vao->addAttributes(layouts);
    setVAO(std::move(vao));
}