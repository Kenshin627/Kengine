#include "animator.h"
#include "animation.h"

Animator::Animator(Animation* animation)
{
	playAnimation(animation);	
}

Animator::~Animator()
{
}

void Animator::playAnimation(Animation* animation)
{
	mCurrentAnimation = animation;
	mCurrentTime = 0.0;	
	mBoneMatrices.reserve(1000);
	for (int i = 0; i < 1000; i++)
	{
		mBoneMatrices.push_back(glm::mat4(1.0f));
	}
}

void Animator::updateAnimation(double deltaTime)
{
	if (!mCurrentAnimation)
	{
		return;
	}
	mCurrentTime += mCurrentAnimation->getSpeed() * deltaTime;
	mCurrentTime = fmod(mCurrentTime, mCurrentAnimation->getDuration());
	auto rootNode = mCurrentAnimation->getRootNode();
	updateBoneMatrix(&rootNode, glm::mat4(1.0));
}

void Animator::updateBoneMatrix(SceneNode* node, glm::mat4 parentTransform)
{
	glm::mat4 nodeTransform = node->transformation;
	auto bone = mCurrentAnimation->getBone(node->name);
	if (bone)
	{
		bone->update(mCurrentTime);
		nodeTransform = bone->getLocalTransform();
	}
	glm::mat4 globalTransform = parentTransform * nodeTransform;
	auto& boneInfo = mCurrentAnimation->getBoneInfo();
	auto iter = boneInfo.find(node->name);
	if (iter != boneInfo.end())
	{
		int id = iter->second.id;
		glm::mat4 offset = iter->second.offset;
		glm::mat4 finalTransform = globalTransform * offset;
		mBoneMatrices[id] = finalTransform;
	}
	for (int i = 0; i < node->childrenCount; i++)
	{
		updateBoneMatrix(&(node->children[i]), globalTransform);
	}
}
