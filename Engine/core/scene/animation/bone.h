#pragma once
#include <string>
#include <vector>
#include <glm.hpp>
#include <assimp/anim.h>
#include <gtc/quaternion.hpp>
#include "typedef.h"

struct PosKeyFrame
{
	glm::vec3 position;
	double	  timeStamp;
};

struct RotKeyFrame
{
	glm::quat rotationQuaternion;
	double	  timeStamp;
};

struct ScaleKeyFrame
{
	glm::vec3 scale;
	double	  timeStamp;
};

class Bone
{
public:
	Bone(const std::string& name, int id, aiNodeAnim* boneNode);
	~Bone();
	void update(double animationTime);
	const glm::mat4& getLocalTransform() const { return mLocalTransform; }
	int getBoneID() const { return mRendererID; }
	const std::string& getBoneName() const { return mName; }
private:
	uint getPositionIndex(double animationTime);
	uint getRotationIndex(double animationTime);
	uint getScaleIndex(double animationTime);
	double getScaleFactor(double lastTimeStamp, double nextTimeStamp, double animationTime);
	glm::mat4 interpolatePosition(double animationTime);
	glm::mat4 interpolateRotation(double animationTime);
	glm::mat4 interpolateScale(double animationTime);
private:
	std::string				   mName;
	int						   mRendererID;	
	glm::mat4				   mLocalTransform;
	uint					   mPosKeyFrameCount;
	uint					   mRotKeyFrameCount;
	uint					   mScaleKeyFrameCount;
	std::vector<PosKeyFrame>   mPosKeyFrames;
	std::vector<RotKeyFrame>   mRotKeyFrames;
	std::vector<ScaleKeyFrame> mScaleKeyFrames;
};