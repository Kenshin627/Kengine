#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "core.h"
#include "typedef.h"
#include "geometry/geometry.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "material/phongMaterial.h"
#include "model.h"

Model::Model(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene =  importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || (scene->mFlags | AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
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
	std::vector<Vertex> vertices;
	std::vector<uint> indices;

	//vertex
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

	//material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		if (!mat)
		{
			KS_CORE_ERROR("material is Null");
			return;
		}
		//diffuse
		aiString dpffTextureName;
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &dpffTextureName);
		//specular
		aiString specTextureName;
		mat->GetTexture(aiTextureType_SPECULAR, 0, &specTextureName);
		//normal
		aiString normalTextureName;
		mat->GetTexture(aiTextureType_NORMALS, 0, &normalTextureName);

		aiString shinessTextureName;
		mat->GetTexture(aiTextureType_SHININESS, 0, &shinessTextureName);
		/*std::shared_ptr<PhongMaterial> material = std::make_shared<PhongMaterial>(
			mTextureDirectory + "/" + std::string(dpffTextureName.C_Str()),
			mTextureDirectory + "/" + std::string(specTextureName.C_Str()),
			mTextureDirectory + "/" + std::string(normalTextureName.C_Str()),
			mTextureDirectory + "/" + std::string(shinessTextureName.C_Str())
		);*/
		
	}
}
