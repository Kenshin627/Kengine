#include "cylinder.h"

Cylinder::Cylinder(float radiusTop, float radiusBottom, float height, int radialSegments, int heightSegments, bool openEnded, float thetaStart, float thetaLength)
	:Geometry(),
	mRadiusTop(radiusTop),
	mRadiusBottom(radiusBottom),
	mHeight(height),
	mRadialSegments(radialSegments),
	mHeightSegments(heightSegments),
	mOpenEnded(openEnded),
	mThetaStart(thetaStart),
	mThetaLength(thetaLength)
{		
	std::vector<Vertex> vertices;
	std::vector<int> indices;
	int index = 0;
	generateTorso(index, vertices, indices);

	if (!mOpenEnded) 
	{
		if (radiusTop > 0)
		{
			generateCap(true, index, vertices, indices);
		}
		if (radiusBottom > 0)
		{
			generateCap(false, index, vertices, indices);
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

void Cylinder::generateTorso(int& index, std::vector<Vertex>& vertices, std::vector<int>& indices)
{

	std::vector<std::vector<int>> indexArray;
	float halfHeight = mHeight / 2.0;

	// this will be used to calculate the normal
	float slope = (mRadiusBottom - mRadiusTop) / (float)mHeight;

	//generate vertices, normals and uvs
	for (int y = 0; y <= mHeightSegments; ++y)
	{
		std::vector<int> indexRow;
		float v = y / (float)mHeightSegments;

		//calculate the radius of the current row
		float radius = v * (mRadiusBottom - mRadiusTop) + mRadiusTop;
		for (int x = 0; x <= mRadialSegments; ++x)
		{
			Vertex vertex;
			float u = x / (float)mRadialSegments;
			float theta = u * mThetaLength + mThetaStart;
			float sinTheta = sin(theta);
			float cosTheta = cos(theta);

			//position
			vertex.Position.x = radius * sinTheta;
			vertex.Position.y = -v * mHeight + halfHeight;
			vertex.Position.z = radius * cosTheta;

			//normal
			vertex.Normal.x = sinTheta;
			vertex.Normal.y = slope;
			vertex.Normal.z = cosTheta;
			vertex.Normal = glm::normalize(vertex.Normal);

			//uv
			vertex.Texcoord.x = u;
			vertex.Texcoord.y = 1 - v;
			vertices.push_back(vertex);

			//save index of vertex in respective row
			indexRow.push_back(index++);
		}

		//now save vertices of the row in our index array
		indexArray.push_back(indexRow);
	}

	//generate indices
	for (int x = 0; x < mRadialSegments; ++x)
	{
		for (int y = 0; y < mHeightSegments; ++y)
		{
			// we use the index array to access the correct indices
			int a = indexArray[y][x];
			int b = indexArray[y + 1][x];
			int c = indexArray[y + 1][x + 1];
			int d = indexArray[y][x + 1];

			// faces
			if (mRadiusTop > 0 || y != 0)
			{
				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(d);
			}

			if (mRadiusBottom > 0 || y != mHeightSegments - 1)
			{
				indices.push_back(b);
				indices.push_back(c);
				indices.push_back(d);
			}
		}
	}
}

void Cylinder::generateCap(bool top, int& index, std::vector<Vertex>& vertices, std::vector<int>& indices)
{
	// save the index of the first center vertex
	int centerIndexStart = index;

	//const uv = new Vector2();
	//const vertex = new Vector3();
	//
	//let groupCount = 0;

	float radius = top? mRadiusTop : mRadiusBottom;
	int sign = top? 1 : -1;

	// first we generate the center vertex data of the cap.
	// because the geometry needs one set of uvs per face,
	// we must generate a center vertex per face/segment
	float halfHeight = mHeight / 2.0;
	for (int x = 1; x <= mRadialSegments; ++x) 
	{
		//vertex
		Vertex vertex;
		vertex.Position.x = 0;
		vertex.Position.y = halfHeight * sign;
		vertex.Position.z = 0;

		// normal
		vertex.Normal.x = 0;
		vertex.Normal.y = sign;
		vertex.Normal.z = 0;

		// uv
		vertex.Texcoord.x = 0.5f;
		vertex.Texcoord.y = 0.5f;

		vertices.push_back(vertex);
		// increase index
		index++;
	}

	// save the index of the last center vertex
	int centerIndexEnd = index;

	// now we generate the surrounding vertices, normals and uvs
	for (int x = 0; x <= mRadialSegments; ++x) 
	{
		float u = x / (float)mRadialSegments;
		float theta = u * mThetaLength + mThetaStart;

		float cosTheta = cos(theta);
		float sinTheta = sin(theta);

		// vertex
		Vertex vertex;
		vertex.Position.x = radius * sinTheta;
		vertex.Position.y = halfHeight * sign;
		vertex.Position.z = radius * cosTheta;

		// normal
		vertex.Normal.x = 0;
		vertex.Normal.y = sign;
		vertex.Normal.z = 0;
		// uv

		vertex.Texcoord.x = (cosTheta * 0.5) + 0.5;
		vertex.Texcoord.y = (sinTheta * 0.5 * sign) + 0.5;
		vertices.push_back(vertex);

		// increase index

		index++;
	}

	// generate indices

	for (int x = 0; x < mRadialSegments; ++x) 
	{
		int c = centerIndexStart + x;
		int i = centerIndexEnd + x;

		if (top) 
		{
			//face top
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(c);
		}
		else 
		{
			//face bottom
			indices.push_back(i+1);
			indices.push_back(i);
			indices.push_back(c);

		}
	}
}


