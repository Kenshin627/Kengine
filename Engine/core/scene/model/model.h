#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

struct BoneInfo
{
	int		  id;
	glm::mat4 offset{1.0};
};

struct Vertex;
class RenderObject;
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
class PhongMaterial;
class PBRMaterial;
class Texture2D;
class Animation;
enum class TextureInternalFormat;
enum class TextureDataFormat;

class Model
{
public:
	Model(const std::string& path);
	~Model();
	const std::vector<std::shared_ptr<RenderObject>>& getRenderList() const;
	const aiScene* getAIScene() const { return mAIScene; }
	void draw();
	uint& getBoneCount() { return mBoneCount; }
	std::unordered_map<std::string, BoneInfo>& getBoneInfo() { return mBoneInfoMap; }
	Animation* getAnimation() { return mAnimation.get(); }
private:  
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	void processBones(const aiScene* scene, aiMesh* mesh, std::vector<Vertex>& vertices);
	std::shared_ptr<Texture2D> processTexture(const aiScene* scene, aiMaterial* mat, aiTextureType texType);
	void getFormatByChannel(int channel, aiTextureType type, TextureInternalFormat& internelFormat, TextureDataFormat& format);
	std::shared_ptr<PhongMaterial> resolveBlinnPhongMaterial(const aiScene* scene, aiMaterial* mat);
	std::shared_ptr<PBRMaterial> reslovePBRMaterial(const aiScene* scene, aiMaterial* mat);
	void setVertexBoneDefaultData(Vertex& v);
private:
	std::vector<std::shared_ptr<RenderObject>> mRenderObjectList;
	std::string mTextureDirectory;
	std::string mFileType;
	std::unordered_map<std::string, BoneInfo> mBoneInfoMap;
	uint mBoneCount{0};
	const aiScene* mAIScene;
	std::unique_ptr<Animation> mAnimation;
};