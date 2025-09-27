#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include "core.h"
#include "typedef.h"
#include "geometry/geometry.h"
#include "graphic/texture/texture2D/texture2D.h"
#include "material/phongMaterial.h"
#include "material/pbrMaterial.h"
#include "model.h"
#include "../renderObject.h"
#include "graphic/texture/textureSystem.h"

static std::vector<char> convertBGRAtoRGBA(const char* bgraData, int width, int height) {
	// �������Ϸ���
	if (!bgraData || width <= 0 || height <= 0) {
		KS_CORE_ERROR("INPUT data ins error!");
	}

	// �����������������ֽ�����ÿ������4�ֽڣ�
	int pixelCount = width * height;
	int byteCount = pixelCount * 4;

	// ��ʼ�����RGBA��������
	std::vector<char> rgbaData(byteCount);

	// ����ÿ�����أ�����B��Rͨ��
	for (int i = 0; i < pixelCount; ++i) {
		// ��ǰ�����������е���ʼ����
		int baseIndex = i * 4;

		// BGRA����ͨ��
		char B = bgraData[baseIndex + 0];  // ��ɫͨ��
		char G = bgraData[baseIndex + 1];  // ��ɫͨ��
		char R = bgraData[baseIndex + 2];  // ��ɫͨ��
		char A = bgraData[baseIndex + 3];  // Alphaͨ��

		// д��RGBA���������B��R��
		rgbaData[baseIndex + 0] = R;  // ��ɫͨ����ǰ
		rgbaData[baseIndex + 1] = G;  // ��ɫͨ������
		rgbaData[baseIndex + 2] = B;  // ��ɫͨ���ں�
		rgbaData[baseIndex + 3] = A;  // Alphaͨ������
	}

	return rgbaData;
}

Model::Model(const std::string& path)
{
	Assimp::Importer importer;
	mFileType = path.substr(path.find_last_of("."));
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
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	std::shared_ptr<Material> material;
	if (mat)
	{
		aiShadingMode shadingMode;
		aiReturn res = mat->Get(AI_MATKEY_SHADING_MODEL, shadingMode);
		
		if (res == aiReturn_SUCCESS)
		{
			switch (shadingMode)
			{
			case aiShadingMode_Flat:
				break;
			case aiShadingMode_Gouraud:
				break;
			case aiShadingMode_Phong:
				break;
			case aiShadingMode_Blinn:
				material = resolveBlinnPhongMaterial(scene, mat);
				break;
			case aiShadingMode_Toon:
				break;
			case aiShadingMode_OrenNayar:
				break;
			case aiShadingMode_Minnaert:
				break;
			case aiShadingMode_CookTorrance:
				break;
			case aiShadingMode_NoShading:
				break;
			case aiShadingMode_Fresnel:
				break;
			case aiShadingMode_PBR_BRDF:
				material = reslovePBRMaterial(scene, mat);
				break;
			case _aiShadingMode_Force32Bit:
				break;
			default:
				break;
			}
		}
	}
	
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
			auto texBuffer	 = texData->pcData;
			auto texelLength = texData->mWidth;
			int width;
			int height;
			int channel;
			const unsigned char* rowData = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texBuffer), texelLength, &width, &height, &channel, 0);
			TextureInternalFormat internalFormat;
			TextureDataFormat format;
			std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
			getFormatByChannel(channel, texType, internalFormat, format);
			texture->loadFromData(width, height, rowData, channel, internalFormat, format, GL_UNSIGNED_BYTE);
			return texture;
		}
		else 
		{
			
			std::string texFilePath = (mTextureDirectory + "/" + std::string(texFileName.C_Str())).c_str();
			bool Srgb = texType == aiTextureType::aiTextureType_DIFFUSE;
			return ts.getTexture(texFilePath, true, Srgb);
		}
		
}

void Model::getFormatByChannel(int channel, aiTextureType type, TextureInternalFormat& internelFormat, TextureDataFormat& format)
{
	if (channel == 4)
	{
		if (type == aiTextureType::aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR)
		{
			internelFormat = TextureInternalFormat::SRGB8ALPHA8;
			format = TextureDataFormat::RGBA;
		}
		else
		{
			internelFormat = TextureInternalFormat::RGBA8;
			format = TextureDataFormat::RGBA;
		}
	}
	else if (channel == 3)
	{
		if (type == aiTextureType::aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR)
		{
			internelFormat = TextureInternalFormat::SRGB8;
			format = TextureDataFormat::RGB;
		}
		else
		{
			internelFormat = TextureInternalFormat::RGB8;
			format = TextureDataFormat::RGB;
		}
	}

}

std::shared_ptr<PhongMaterial> Model::resolveBlinnPhongMaterial(const aiScene* scene, aiMaterial* mat)
{
	BlinnPhongMaterialSpecification spec;
	if (mat)
	{
		//texs
		spec.diffuseMap = processTexture(scene, mat, aiTextureType_DIFFUSE);
		spec.specularMap = processTexture(scene, mat, aiTextureType_SPECULAR);
		spec.normalMap = processTexture(scene, mat, aiTextureType_HEIGHT);
		spec.shinessMap = processTexture(scene, mat, aiTextureType_SHININESS);
		spec.alphaMap = processTexture(scene, mat, aiTextureType_OPACITY);


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
	return std::make_shared<PhongMaterial>(spec);
}

std::shared_ptr<PBRMaterial> Model::reslovePBRMaterial(const aiScene* scene, aiMaterial* mat)
{
	PBRMaterialSpecification spec;
	if (mat)
	{
		//texs
		spec.albedoMap = processTexture(scene, mat, aiTextureType_BASE_COLOR);
		spec.metallicMap = processTexture(scene, mat, aiTextureType_METALNESS);
		spec.roughnessMap = processTexture(scene, mat, aiTextureType_DIFFUSE_ROUGHNESS);
		spec.normalMap = processTexture(scene, mat, aiTextureType_NORMAL_CAMERA);
		spec.heightMap = processTexture(scene, mat, aiTextureType_HEIGHT);


		//colors
		aiColor3D albedo;
		aiReturn diffRes = mat->Get(AI_MATKEY_BASE_COLOR, albedo);
		if (diffRes == aiReturn_SUCCESS)
		{
			spec.albedoColor.r = albedo.r;
			spec.albedoColor.g = albedo.g;
			spec.albedoColor.b = albedo.b;
		}

		float metallic;
		aiReturn metalRes = mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
		if (metalRes == aiReturn_SUCCESS)
		{
			spec.metallic = metallic;
		}

		float roughness;
		aiReturn roughnessRes = mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
		if (roughnessRes == aiReturn_SUCCESS)
		{
			spec.roughness = roughnessRes;
		}
	}
	return std::make_shared<PBRMaterial>(spec);
}
