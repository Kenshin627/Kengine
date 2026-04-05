#include "torus.h"

Torus::Torus(float radius, float tube, int radialSegments, int tubularSegments, float arc, float thetaStart, float thetaLength)
	:Geometry(),
	mRadius(radius),
	mTube(tube),
	mRadialSegments(radialSegments),
	mTubularSegments(tubularSegments),
	mArc(arc),
	mThetaStart(thetaStart),
	mThetaLength(thetaLength)
{
	buildGeometry();
}

void Torus::buildGeometry()
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;
	// generate vertices, normals and uvs
	for (int j = 0; j <= mRadialSegments; ++j)
	{
		float v = mThetaStart + (j / (float)mRadialSegments) * mThetaLength;
		for (int i = 0; i <= mTubularSegments; ++i)
		{
			float u = (float)i / mTubularSegments * mArc;

			// vertex
			Vertex vertex;
			vertex.Position.x = (mRadius + mTube * cos(v)) * cos(u);
			vertex.Position.y = (mRadius + mTube * cos(v)) * sin(u);
			vertex.Position.z = mTube * sin(v);

			// normal
			glm::vec3 center;
			center.x = mRadius * cos(u);
			center.y = mRadius * sin(u);
			vertex.Normal = vertex.Position - center;
			vertex.Normal = glm::normalize(vertex.Normal);

			vertex.Texcoord.x = (float)i / mTubularSegments;
			vertex.Texcoord.y = (float)j / mRadialSegments;
			vertices.push_back(vertex);
		}

	}

	// generate indices

	for (int j = 1; j <= mRadialSegments; ++j)
	{
		for (int i = 1; i <= mTubularSegments; ++i)
		{
			// indices
			int a = (mTubularSegments + 1) * j + i - 1;
			int b = (mTubularSegments + 1) * (j - 1) + i - 1;
			int c = (mTubularSegments + 1) * (j - 1) + i;
			int d = (mTubularSegments + 1) * j + i;

			// faces
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(d);
			indices.push_back(b);
			indices.push_back(c);
			indices.push_back(d);
		}
	}

	// build geometry
	std::unique_ptr<VertexArray> vao = std::make_unique< VertexArray>(indices.size(), GL_TRIANGLES);
	uint vboId = vao->buildVertexBuffer(sizeof(Vertex) * vertices.size(), (void*)vertices.data(), GL_DYNAMIC_STORAGE_BIT);
	vao->buildIndexBuffer(indices.data(), sizeof(uint) * indices.size(), GL_UNSIGNED_INT, GL_DYNAMIC_STORAGE_BIT);
	uint stride = sizeof(Vertex);
	std::initializer_list<AttributeLayout> layouts = {
		{0, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Position), 0},
		{1, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Normal), 0},
		{2, vboId, 0, 0, stride, 2, GL_FLOAT, false, offsetof(Vertex, Texcoord), 0},
		{3, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Tangent),0}
	};
	vao->addAttributes(layouts);
	setVAO(std::move(vao));
}