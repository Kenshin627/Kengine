
#define GLM_ENABLE_EXPERIMENTAL
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include "bone.h"
#include "scene/assimpGlmHelper.h"

Bone::Bone(const std::string& name, int id, aiNodeAnim* boneNode)
	:mName(name),
	mRendererID(id),
	mLocalTransform(1.0f),
	mPosKeyFrameCount(0),
	mRotKeyFrameCount(0),
	mScaleKeyFrameCount(0)
{
	mPosKeyFrameCount = boneNode->mNumPositionKeys;
	mRotKeyFrameCount = boneNode->mNumRotationKeys;
	mScaleKeyFrameCount = boneNode->mNumScalingKeys;

	for (int i = 0; i < mPosKeyFrameCount; i++)
	{
		PosKeyFrame keyFrame;
		keyFrame.position = AssimpGLMHelpers::GetGLMVec(boneNode->mPositionKeys[i].mValue);
		keyFrame.timeStamp = boneNode->mPositionKeys[i].mTime;
		mPosKeyFrames.push_back(keyFrame);
	}

	for (int i = 0; i < mRotKeyFrameCount; i++)
	{
		RotKeyFrame keyFrame;
		keyFrame.rotationQuaternion = AssimpGLMHelpers::GetGLMQuat(boneNode->mRotationKeys[i].mValue);
		keyFrame.timeStamp = boneNode->mRotationKeys[i].mTime;
		mRotKeyFrames.push_back(keyFrame);
	}

	for (int i = 0; i < mScaleKeyFrameCount; i++)
	{
		ScaleKeyFrame keyFrame;
		keyFrame.scale = AssimpGLMHelpers::GetGLMVec(boneNode->mScalingKeys[i].mValue);
		keyFrame.timeStamp = float(boneNode->mScalingKeys[i].mTime);
		mScaleKeyFrames.push_back(keyFrame);
	}
}

Bone::~Bone()
{
}

void Bone::update(double animationTime)
{
	glm::mat4 translation = interpolatePosition(animationTime);
	glm::mat4 rotation = interpolateRotation(animationTime);
	glm::mat4 scale = interpolateScale(animationTime);
	mLocalTransform = translation * rotation * scale;
}

uint Bone::getPositionIndex(double animationTime)
{
	for (int i = 0; i < mPosKeyFrameCount - 1; i++)
	{
		if (animationTime < mPosKeyFrames[i + 1].timeStamp)
		{
			return i;
		}
	}
}

uint Bone::getRotationIndex(double animationTime)
{
	for (int i = 0; i < mRotKeyFrameCount - 1; i++)
	{
		if (animationTime < mRotKeyFrames[i + 1].timeStamp)
		{
			return i;
		}
	}
}

uint Bone::getScaleIndex(double animationTime)
{
	for (int i = 0; i < mScaleKeyFrameCount - 1; i++)
	{
		if (animationTime < mScaleKeyFrames[i + 1].timeStamp)
		{
			return i;
		}
	}
}

double Bone::getScaleFactor(double lastTimeStamp, double nextTimeStamp, double animationTime)
{
	return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

glm::mat4 Bone::interpolatePosition(double animationTime)
{
	if (mPosKeyFrameCount == 1)
	{
		return glm::translate(glm::mat4(1.0f), mPosKeyFrames[0].position);
	}
	uint posIndex = getPositionIndex(animationTime);
	uint nextPosIndex = posIndex + 1;
	/*if (posIndex == nextPosIndex)
	{
		return glm::translate(glm::mat4(1.0f), mPosKeyFrames[posIndex].position);
	}*/
	double scaleFactor = getScaleFactor(mPosKeyFrames[posIndex].timeStamp, mPosKeyFrames[nextPosIndex].timeStamp, animationTime);
	glm::vec3 finalPosition = glm::mix(mPosKeyFrames[posIndex].position, mPosKeyFrames[nextPosIndex].position, scaleFactor);
	return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(double animationTime)
{
	if (mRotKeyFrameCount == 1)
	{
		glm::quat rotation = glm::normalize(mRotKeyFrames[0].rotationQuaternion);
		return glm::toMat4(rotation);
	}
	uint rotIndex = getRotationIndex(animationTime);
	uint nextRotIndex = rotIndex + 1;
	/*if (rotIndex == nextRotIndex)
	{
		glm::quat rotation = glm::normalize(mRotKeyFrames[rotIndex].rotationQuaternion);
		return glm::toMat4(rotation);
	}*/
	double scaleFactor = getScaleFactor(mRotKeyFrames[rotIndex].timeStamp, mRotKeyFrames[nextRotIndex].timeStamp, animationTime);
	glm::quat finalRotation = glm::slerp(mRotKeyFrames[rotIndex].rotationQuaternion, mRotKeyFrames[nextRotIndex].rotationQuaternion, (float)scaleFactor);
	return glm::toMat4(finalRotation);
}

glm::mat4 Bone::interpolateScale(double animationTime)
{
	if (mScaleKeyFrameCount == 1)
	{
		return glm::scale(glm::mat4(1.0f), mScaleKeyFrames[0].scale);
	}
	uint scaleIndex = getScaleIndex(animationTime);
	uint nextScaleIndex = scaleIndex + 1;
	/*if (scaleIndex == nextScaleIndex)
	{
		return glm::scale(glm::mat4(1.0f), mScaleKeyFrames[scaleIndex].scale);
	}*/
	double scaleFactor = getScaleFactor(mScaleKeyFrames[scaleIndex].timeStamp, mScaleKeyFrames[nextScaleIndex].timeStamp, animationTime);
	glm::vec3 finalScale = glm::mix(mScaleKeyFrames[scaleIndex].scale, mScaleKeyFrames[nextScaleIndex].scale, (float)scaleFactor);
	return glm::scale(glm::mat4(1.0f), finalScale);
}
