#pragma once
#include <vector>
#include <glm.hpp>

class Animation;
class SceneNode;
class Animator
{
public:
	Animator(Animation* animation);
	Animator() = default;
	~Animator();
	void playAnimation(Animation* animation);
	void updateAnimation(double deltaTime);
	void updateBoneMatrix(SceneNode* node, glm::mat4 parentTransform);
	const std::vector<glm::mat4>& getBoneMatrices() const { return mBoneMatrices; }
private:
	Animation*			    mCurrentAnimation	{nullptr};
	double					mCurrentTime;
	std::vector<glm::mat4>  mBoneMatrices;
};