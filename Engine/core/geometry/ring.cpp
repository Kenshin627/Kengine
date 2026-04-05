#include "ring.h"

Ring::Ring(float innerRadius, float outerRadius, int thetaSegments, int phiSegments, float thetaStart, float thetaLength)
	:Geometry(),
	mInnerRadius(innerRadius),
	mOuterRadius(outerRadius),
	mThetaSegments(std::max(3, thetaSegments)),
	mPhiSegments(std::max(1, phiSegments)),
	mThetaStart(thetaStart),
	mThetaLength(thetaLength)
{
	buildGeometry();
}

void Ring::buildGeometry()
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	float radius = mInnerRadius;
	float radiusStep = ((mOuterRadius - mInnerRadius) / (float)mPhiSegments);

	// generate vertices, normals and uvs
	for (int j = 0; j <= mPhiSegments; ++j) 
	{
		for (int i = 0; i <= mThetaSegments; ++i)
		{
			// values are generate from the inside of the ring to the outside
			float segment = mThetaStart + i / (float)(mThetaSegments * mThetaLength);

			Vertex vertex;
			vertex.Position.x = radius * std::cos(segment);
			vertex.Position.y = 0;
			vertex.Position.z = radius * std::sin(segment);

			vertex.Normal.x = 0;
			vertex.Normal.y = 1;
			vertex.Normal.z = 0;

			vertex.Texcoord.x = (vertex.Position.x / mOuterRadius + 1) / 2;
			vertex.Texcoord.y = (vertex.Position.y / mOuterRadius + 1) / 2;
			vertices.push_back(vertex);
		}

		// increase the radius for next row of vertices
		radius += radiusStep;

	}

	// indices

	for (int j = 0; j < mPhiSegments; ++j)
	{
		int thetaSegmentLevel = j * (mThetaSegments + 1);
		for (int i = 0; i < mThetaSegments; ++i) 
		{
			int segment = i + thetaSegmentLevel;
			int a = segment;
			int b = segment + mThetaSegments + 1;
			int c = segment + mThetaSegments + 2;
			int d = segment + 1;

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
