#pragma once
#include "bone.h"
#include "../model/model.h"

struct SceneNode
{
	std::string name;
	glm::mat4   transformation;
	std::vector<SceneNode> children;
	uint childrenCount;
};

class Animation
{
public:
	Animation(Model* model);
	~Animation() = default;
	float getDuration() const { return mDuration; }
	float getSpeed() const { return mSpeed; }
	const SceneNode& getRootNode() const { return mRootNode; }
	Bone* getBone(const std::string& name);
	std::unordered_map<std::string, BoneInfo>& getBoneInfo() { return mBoneInfo; }
private:
	void processHierarchy(SceneNode& dest, aiNode* src);
	void processBones(Model* model);
private:
	float mDuration;
	float mSpeed;
	std::vector<Bone> mBones;
	SceneNode mRootNode;
	std::unordered_map<std::string, BoneInfo> mBoneInfo;
};