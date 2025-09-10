#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "core.h"
#include "typedef.h"
#include "geometry/geometry.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "material/phongMaterial.h"
#include "model.h"
#include "../renderObject.h"

Model::Model(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene =  importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	if (!scene || !(scene->mFlags | AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
	{
		KS_CORE_ERROR("parse model error");
		return;
	}
	mTextureDirectory = path.substr(0, path.find_last_of("/"));
	processNode(scene->mRootNode, scene);
}

Model::~Model()
{
}

const std::vector<std::shared_ptr<RenderObject>>& Model::getRenderList() const
{
	return mRenderObjectList;
}

void Model::draw()
{
	for (auto& renderObject : mRenderObjectList)
	{
		renderObject->beginDraw();
		renderObject->draw();
		renderObject->endDraw();
	}
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	if (!node || !scene)
	{
		KS_CORE_ERROR("parse Node error, node or scene is Null");
		return;
	}
	//meshes
	uint meshNum = node->mNumMeshes;
	for (int i = 0; i < meshNum; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		if (!mesh)
		{
			KS_CORE_ERROR("mesh is Null");
			return;
		}
		processMesh(mesh, scene);
	}
	//nodes
	uint nodeNum = node->mNumChildren;
	for (int i = 0; i < nodeNum; i++)
	{
		aiNode* currentNode = node->mChildren[i];
		if (!currentNode)
		{
			KS_CORE_ERROR("node is Null");
			return;
		}
		processNode(currentNode, scene);
	}
}

void Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::shared_ptr<Geometry> geometry = std::make_shared<Geometry>();
	//vertex
	std::vector<Vertex> vertices;
	if (!mesh)
	{
		KS_CORE_ERROR("mesh is Null");
		return;
	}
	uint vertexNum = mesh->mNumVertices;
	vertices.reserve(vertexNum);
	for (int i = 0; i < vertexNum; i++)
	{
		Vertex vertex;
		aiVector3D pos = mesh->mVertices[i];
		vertex.Position.x = pos.x;
		vertex.Position.y = pos.y;
		vertex.Position.z = pos.z;
		aiVector3D normal = mesh->mNormals[i];
		vertex.Normal.x = normal.x;
		vertex.Normal.y = normal.y;
		vertex.Normal.z = normal.z;
		auto texCoord = mesh->mTextureCoords[0][i];
		vertex.Texcoord.s = texCoord.x;
		vertex.Texcoord.t = texCoord.y;

		aiVector3D tangent = mesh->mTangents[i];
		vertex.tangent.x = tangent.x;
		vertex.tangent.y = tangent.y;
		vertex.tangent.z = tangent.z;

		aiVector3D bitangent = mesh->mBitangents[i];
		vertex.bitangent.x = bitangent.x;
		vertex.bitangent.y = bitangent.y;
		vertex.bitangent.z = bitangent.z;
		vertices.push_back(vertex);
	}

	//indices
	std::vector<uint> indices;
	uint faceNum = mesh->mNumFaces;
	for (int i = 0; i < faceNum; i++)
	{
		aiFace face = mesh->mFaces[i];
		uint faceIndiceNum = face.mNumIndices;
		for (int j = 0; j < faceIndiceNum; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	std::unique_ptr<VertexArray> vao = std::make_unique< VertexArray>(indices.size(), GL_TRIANGLES);
	uint vboId = vao->buildVertexBuffer(sizeof(Vertex) * vertices.size(), (void*)vertices.data(), 0);
	vao->buildIndexBuffer(indices.data(), sizeof(uint) * indices.size(), GL_UNSIGNED_INT, 0);
	uint stride = sizeof(Vertex);
	//glm::vec3 Position;
	//glm::vec3 Normal;
	//glm::vec2 Texcoord;
	//glm::vec3 tangent;
	//glm::vec3 bitangent;
	std::initializer_list<AttributeLayout> layouts = {
		{0, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Position), 0},
		{1, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Normal), 0},
		{2, vboId, 0, 0, stride, 2, GL_FLOAT, false, offsetof(Vertex, Texcoord), 0},
		{3, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, tangent),0},
		{4, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, bitangent), 0}
	};
	vao->addAttributes(layouts);
	geometry->setVAO(std::move(vao));

	//material	
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	if (!mat)
	{
		KS_CORE_ERROR("material is Null");
		return;
	}
	//diffuse
	auto diffTex = processTexture(mat, aiTextureType_DIFFUSE);

	//specular
	auto specTex = processTexture(mat, aiTextureType_SPECULAR);

	//normal
	auto normalTex = processTexture(mat, aiTextureType_HEIGHT);

	std::shared_ptr<PhongMaterial> material = std::make_shared<PhongMaterial>(diffTex, specTex, 128, normalTex);
	mRenderObjectList.push_back(std::make_shared<RenderObject>(geometry, material));
}

std::shared_ptr<Texture2D> Model::processTexture(aiMaterial* mat, aiTextureType texType)
{
	aiString texFileName;
	mat->GetTexture(texType, 0, &texFileName);
	std::shared_ptr<Texture2D> texture;
	std::string texFilePath = (mTextureDirectory + "/" + std::string(texFileName.C_Str())).c_str();
	auto iter = mTextures.find(texFilePath);
	if (iter != mTextures.cend())
	{
		texture = iter->second;
	}
	else
	{
		texture = std::make_shared<Texture2D>();
		texture->loadFromFile(texFilePath.c_str());
		mTextures.insert({ texFilePath, texture });
	}
	return texture;
}
