#pragma once
#include <memory>
#include <vector>
#include <string>

class RenderObject;
struct aiNode;
struct aiScene;
struct aiMesh;

class Model
{
public:
	Model(const std::string& path);
	~Model();
private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
private:
	std::vector<std::shared_ptr<RenderObject>> mRenderObjectList;
	std::string mTextureDirectory;
};