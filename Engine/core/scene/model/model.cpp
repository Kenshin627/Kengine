#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include "core.h"
#include "typedef.h"
#include "geometry/geometry.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "material/phongMaterial.h"
#include "model.h"
#include "../renderObject.h"
#include "graphic/texture/textureSystem.h"

static std::vector<char> convertBGRAtoRGBA(const char* bgraData, int width, int height) {
	// 检查输入合法性
	if (!bgraData || width <= 0 || height <= 0) {
		KS_CORE_ERROR("INPUT data ins error!");
	}

	// 计算总像素数和总字节数（每个像素4字节）
	int pixelCount = width * height;
	int byteCount = pixelCount * 4;

	// 初始化输出RGBA数据容器
	std::vector<char> rgbaData(byteCount);

	// 遍历每个像素，交换B和R通道
	for (int i = 0; i < pixelCount; ++i) {
		// 当前像素在数组中的起始索引
		int baseIndex = i * 4;

		// BGRA输入通道
		char B = bgraData[baseIndex + 0];  // 蓝色通道
		char G = bgraData[baseIndex + 1];  // 绿色通道
		char R = bgraData[baseIndex + 2];  // 红色通道
		char A = bgraData[baseIndex + 3];  // Alpha通道

		// 写入RGBA输出（交换B和R）
		rgbaData[baseIndex + 0] = R;  // 红色通道在前
		rgbaData[baseIndex + 1] = G;  // 绿色通道不变
		rgbaData[baseIndex + 2] = B;  // 蓝色通道在后
		rgbaData[baseIndex + 3] = A;  // Alpha通道不变
	}

	return rgbaData;
}

Model::Model(const std::string& path)
{
	Assimp::Importer importer;
	mFileType = path.substr(path.find_last_of("/"));
	uint importFlags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
	if (mFileType == "Glb")
	{
		
	}
	else if (mFileType == "obj")
	{
		importFlags |= aiProcess_FlipUVs;
	}
	
	const aiScene* scene =  importer.ReadFile(path, importFlags);
	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
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
	std::initializer_list<AttributeLayout> layouts = {
		{0, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Position), 0},
		{1, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, Normal), 0},
		{2, vboId, 0, 0, stride, 2, GL_FLOAT, false, offsetof(Vertex, Texcoord), 0},
		{3, vboId, 0, 0, stride, 3, GL_FLOAT, false, offsetof(Vertex, tangent),0}
	};
	vao->addAttributes(layouts);
	geometry->setVAO(std::move(vao));

	//material	
	BlinnPhongMaterialSpecification spec;
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	if (mat)
	{
		//texs
		spec.diffuseMap = processTexture(scene, mat, aiTextureType_DIFFUSE);
		spec.specularMap = processTexture(scene, mat, aiTextureType_SPECULAR);
		spec.normalMap = processTexture(scene, mat, aiTextureType_HEIGHT);
		spec.shinessMap = processTexture(scene, mat, aiTextureType_SHININESS);

		//colors
		aiColor3D diffColor;
		aiReturn diffRes = mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffColor);
		if (diffRes == aiReturn_SUCCESS)
		{
			spec.diffuseColor.r = diffColor.r;
			spec.diffuseColor.g = diffColor.g;
			spec.diffuseColor.b = diffColor.b;
		}

		aiColor3D specColor;
		aiReturn specRes = mat->Get(AI_MATKEY_COLOR_SPECULAR, specColor);
		if (specRes == aiReturn_SUCCESS)
		{
			spec.specularColor.r = specColor.r;
			spec.specularColor.g = specColor.g;
			spec.specularColor.b = specColor.b;
		}

		float shiness;
		aiReturn shinessRes = mat->Get(AI_MATKEY_SHININESS, shiness);
		if (shinessRes == aiReturn_SUCCESS)
		{
			spec.shiness = shiness;
		}
	}
	std::shared_ptr<PhongMaterial> material = std::make_shared<PhongMaterial>(spec);
	mRenderObjectList.push_back(std::make_shared<RenderObject>("", geometry, material));
}

std::shared_ptr<Texture2D> Model::processTexture(const aiScene* scene, aiMaterial* mat, aiTextureType texType)
{
		
		TextureSystem& ts = TextureSystem::getInstance();;
		aiString texFileName;
		mat->GetTexture(texType, 0, &texFileName);
		if (texFileName.Empty())
		{
			return nullptr;
		}
		else if (texFileName.C_Str()[0] == '*')
		{
			uint tex = atoi(&texFileName.C_Str()[1]);
			//row puxeL Data need convert using stb_image
			aiTexture* texData = scene->mTextures[tex];
			auto texBuffer = &(texData->pcData)[0].b;
			auto texelLength = texData->mWidth;
			int width;
			int height;
			int channel;
			const unsigned char* rowData = stbi_load_from_memory(texBuffer, texelLength, &width, &height, &channel, 0);

			std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
			texture->loadFromData(width, height, rowData, 4, TextureInternalFormat::RGBA8, TextureDataFormat::RGBA, GL_UNSIGNED_BYTE);
			return texture;
		}
		else 
		{
			
			std::string texFilePath = (mTextureDirectory + "/" + std::string(texFileName.C_Str())).c_str();
			return ts.getTexture(texFilePath);
		}
		
}
