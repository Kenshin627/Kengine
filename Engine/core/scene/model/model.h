#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class RenderObject;
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
class Texute2D;

class Model
{
public:
	Model(const std::string& path);
	~Model();
	const std::vector<std::shared_ptr<RenderObject>>& getRenderList() const;
	void draw();
private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	std::shared_ptr<Texture2D> processTexture(const aiScene* scene, aiMaterial* mat, aiTextureType texType);
private:
	std::vector<std::shared_ptr<RenderObject>> mRenderObjectList;
	std::string mTextureDirectory;
	std::string mFileType;
};