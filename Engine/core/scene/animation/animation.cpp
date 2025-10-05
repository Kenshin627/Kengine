#include <assimp/scene.h>
#include "scene/assimpGlmHelper.h"
#include "animation.h"
#include "core.h"

Animation::Animation(Model* model)
	:mDuration(0),
	 mSpeed(0)
{
	const aiScene* scene = model->getAIScene();
	if (!scene->mRootNode)
	{
		KS_CORE_ERROR("RootNode is Null");
		return;
	}
	aiNode* root = scene->mRootNode;
	processBones(model);
	processHierarchy(mRootNode, root);
}

Bone* Animation::getBone(const std::string& name)
{
	for (auto& bone : mBones)
	{
		if (bone.getBoneName() == name)
		{
			return &bone;
		}
	}
	return nullptr;
}

void Animation::processHierarchy(SceneNode& dest, aiNode* src)
{
	dest.name = src->mName.C_Str();
	dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
	dest.childrenCount = src->mNumChildren;
	for (int i = 0; i < src->mNumChildren; i++)
	{
		SceneNode child;
		processHierarchy(child, src->mChildren[i]);
		dest.children.push_back(child);
	}
}

void Animation::processBones(Model* model)
{
	auto scene = model->getAIScene();
	if (!scene->HasAnimations())
	{
		return;
	}
	aiAnimation* animation = scene->mAnimations[0];
	mDuration = animation->mDuration;
	mSpeed = animation->mTicksPerSecond;
	uint numChannel = animation->mNumChannels;
	auto boneCount = model->getBoneCount();
	auto& boneInfoMap = model->getBoneInfo();
	for (int i = 0; i < numChannel; i++)
	{
		aiNodeAnim* c = animation->mChannels[i];
		if (boneInfoMap.find(c->mNodeName.C_Str()) == boneInfoMap.end())
		{
			boneInfoMap[c->mNodeName.C_Str()].id = boneCount;
			boneCount++;
		}
		mBones.push_back(Bone(c->mNodeName.C_Str(), boneInfoMap[c->mNodeName.C_Str()].id, c));
	}
	mBoneInfo = boneInfoMap;
}
