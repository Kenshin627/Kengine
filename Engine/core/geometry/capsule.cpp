#include "capsule.h"
#define PI 3.14159265358979

Capsule::Capsule(float radius, float height, int capSegments, int radialSegments, int heightSegments)
	:mRadius(radius),
	mHeight(height),
	mCapSegments(capSegments),
	mRadialSegments(std::max(3, radialSegments)),
	mHeightSegments(heightSegments)
{
	buildGeometry();
}

void Capsule::buildGeometry()
{
	float halfHeight = mHeight / 2.0f;
	float capArcLength = (PI / 2) * mRadius;
	float cylinderPartLength = mHeight;
	float totalArcLength = 2 * capArcLength + cylinderPartLength;

	int numVerticalSegments = mCapSegments * 2 + mHeightSegments;
	int verticesPerRow = mRadialSegments + 1;

	// generate vertices, normals, and uvs
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	for (int iy = 0; iy <= numVerticalSegments; ++iy) 
	{

		float currentArcLength = 0;
		float profileY = 0;
		float profileRadius = 0;
		float normalYComponent = 0;

		if (iy <= mCapSegments) 
		{
			// bottom cap
			float segmentProgress = iy / (float)mCapSegments;
			float angle = (segmentProgress * PI) / 2;
			profileY = -halfHeight - mRadius * cos(angle);
			profileRadius = mRadius * sin(angle);
			normalYComponent = -mRadius * cos(angle);
			currentArcLength = segmentProgress * capArcLength;

		}
		else if (iy <= mCapSegments + mHeightSegments) 
		{
			// middle section
			float segmentProgress = (iy - mCapSegments) / float(mHeightSegments);
			profileY = -halfHeight + segmentProgress * mHeight;
			profileRadius = mRadius;
			normalYComponent = 0;
			currentArcLength = capArcLength + segmentProgress * cylinderPartLength;

		}
		else 
		{
			// top cap
			float segmentProgress = (iy - mCapSegments - mHeightSegments) / float(mCapSegments);
			float angle = (segmentProgress * PI) / 2;
			profileY = halfHeight + mRadius * sin(angle);
			profileRadius = mRadius * cos(angle);
			normalYComponent = mRadius * sin(angle);
			currentArcLength = capArcLength + cylinderPartLength + segmentProgress * capArcLength;
		}

		float v = std::max(0.0f, std::min(1.0f, currentArcLength / totalArcLength));


		// special case for the poles
		float uOffset = 0;

		if (iy == 0) 
		{
			uOffset = 0.5f / mRadialSegments;
		}
		else if (iy == numVerticalSegments) 
		{
			uOffset = -0.5f / mRadialSegments;
		}

		for (int ix = 0; ix <= mRadialSegments; ++ix) 
		{

			float u = ix / float(mRadialSegments);
			float theta = u * PI * 2;

			float sinTheta = sin(theta);
			float cosTheta = cos(theta);

			// vertex
			Vertex vertex;
			glm::vec3 position;
			position.x = -profileRadius * cosTheta;
			position.y = profileY;
			position.z = profileRadius * sinTheta;
			
			// normal
			glm::vec3 normal;
			normal.x = -profileRadius * cosTheta;
			normal.y =  normalYComponent;
			normal.z =  profileRadius * sinTheta;

			normal = glm::normalize(normal);
			
			glm::vec2 uv = { u + uOffset, v };

			vertex.Position = position;
			vertex.Normal = normal;
			vertex.Texcoord = uv;
			vertex.Tangent = glm::vec3(sinTheta, 0.0f, cosTheta); 
			vertices.push_back(vertex);
		}

		if (iy > 0) 
		{
			int prevIndexRow = (iy - 1) * verticesPerRow;
			for (int ix = 0; ix < mRadialSegments; ++ix) {

				int i1 = prevIndexRow + ix;
				int i2 = prevIndexRow + ix + 1;
				int i3 = iy * verticesPerRow + ix;
				int i4 = iy * verticesPerRow + ix + 1;
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);

				indices.push_back(i2);
				indices.push_back(i4);
				indices.push_back(i3);
			}
		}
	}

	// build geometry
	std::unique_ptr<VertexArray> vao = std::make_unique< VertexArray>(indices.size(), GL_TRIANGLES);
	uint vboId = vao->buildVertexBuffer(sizeof(Vertex) * vertices.size(), (void*)vertices.data(), GL_DYNAMIC_STORAGE_BIT);
	vao->buildIndexBuffer(indices.data(), sizeof(uint)* indices.size(), GL_UNSIGNED_INT, GL_DYNAMIC_STORAGE_BIT);
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
