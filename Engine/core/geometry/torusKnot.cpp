#include "torusKnot.h"

TorusKnot::TorusKnot(float radius, float tube, int tubularSegments, int radialSegments, int p, int q)
	:Geometry(),
	mRadius(radius),
	mTube(tube),
	mTubularSegments(tubularSegments),
	mRadialSegments(radialSegments),
	mP(p),
	mQ(q)
{
	buildGeometry();
}

void TorusKnot::buildGeometry()
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	glm::vec3 P1;
	glm::vec3 P2;

	glm::vec3 B;
	glm::vec3 T;
	glm::vec3 N;

	// generate vertices, normals and uvs
	for (int i = 0; i <= mTubularSegments; ++i) 
	{
		// the radian "u" is used to calculate the position on the torus curve of the current tubular segment
		float u = (float)i / mTubularSegments * mP * PI * 2;

		// now we calculate two points. P1 is our current position on the curve, P2 is a little farther ahead.
		// these points are used to create a special "coordinate space", which is necessary to calculate the correct vertex positions
		calculatePositionOnCurve(u, P1);
		calculatePositionOnCurve(u + 0.01f, P2);

		// calculate orthonormal basis
		T = P2 - P1;
		N = P2 + P1;
		B = glm::cross(T, N);
		N = glm::cross(B, T);

		// normalize B, N. T can be ignored, we don't use it
		B = glm::normalize(B);
		N = glm::normalize(N);

		for (int j = 0; j <= mRadialSegments; ++j)
		{
			// now calculate the vertices. they are nothing more than an extrusion of the torus curve.
			// because we extrude a shape in the xy-plane, there is no need to calculate a z-value.
			float v = (float)j / mRadialSegments * PI * 2;
			float cx = -mTube * cos(v);
			float cy = mTube * sin(v);

			//now calculate the final vertex position.
			//first we orient the extrusion with our basis vectors, then we add it to the current position on the curve
			Vertex vertex;
			vertex.Position.x = P1.x + (cx * N.x + cy * B.x);
			vertex.Position.y = P1.y + (cx * N.y + cy * B.y);
			vertex.Position.z = P1.z + (cx * N.z + cy * B.z);
		
			glm::vec3 n = glm::vec3(vertex.Position.x - P1.x, vertex.Position.y - P1.y, vertex.Position.z - P1.z);
			n = glm::normalize(n);
			vertex.Normal.x = n.x;
			vertex.Normal.y = n.y;
			vertex.Normal.z = n.z;

			vertex.Texcoord.x = i / (float)mTubularSegments;
			vertex.Texcoord.y = j / (float)mRadialSegments;
			vertices.push_back(vertex);
		}
	}

	// generate indices
	for (int j = 1; j <= mTubularSegments; ++j)
	{
		for (int i = 1; i <= mRadialSegments; ++i)
		{
			// indices
			float a = (mRadialSegments + 1) * (j - 1) + (i - 1);
			float b = (mRadialSegments + 1) * j + (i - 1);
			float c = (mRadialSegments + 1) * j + i;
			float d = (mRadialSegments + 1) * (j - 1) + i;

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

void TorusKnot::calculatePositionOnCurve(float u, glm::vec3& position) 
{
	float cu = cos(u);
	float su = sin(u);
	float quOverP = (float)mQ / mP * u;
	float cs = cos(quOverP);

	position.x = mRadius * (2 + cs) * 0.5 * cu;
	position.y = mRadius * (2 + cs) * su * 0.5;
	position.z = mRadius * sin(quOverP) * 0.5;
}
