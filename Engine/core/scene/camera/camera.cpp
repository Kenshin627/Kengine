#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "camera.h"

Camera::Camera()  
{
	
}

Camera::Camera(const glm::vec3& pos, float fov, int width ,int height, float near, float far)
	:mPosition(pos),
	 mFovAngle(fov),
	 mAspectRatio(static_cast<float>(width) / static_cast<float>(height)),
	 mViewport(width, height)
{
	mClippingRange[0] = near;
	mClippingRange[1] = far;
}

Camera::~Camera() { }

const glm::mat4& Camera::getViewMatrix()
{
	if (mViewMatrixDirty)
	{
		computeViewMatrix();
		mViewMatrixDirty = false;
	}
	return mViewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix()
{
	if (mProjectionMatrixDirty)
	{
		computeProjectionMatrix();
		mProjectionMatrixDirty = false;
	}
	return mProjectionMatrix;
}

const glm::mat4& Camera::getViewProjectionMatrix()
{
	if (mViewProjectionMatrixDirty)
	{
		mViewProjectionMatrix = getProjectionMatrix() * getViewMatrix();
		mViewProjectionMatrixDirty = false;
	}
	return mViewProjectionMatrix;
}

float Camera::getNear() const
{
	return mClippingRange[0];
}

float Camera::getFar() const
{
	return mClippingRange[1];
}

float Camera::getAspectRatio() const
{
	return mAspectRatio;
}

const glm::vec2 Camera::getViewport() const
{
	return mViewport;
}

const glm::vec3& Camera::getPosition() const
{
	return mPosition;
}

const glm::vec3& Camera::getCenter() const
{
	return mCenter;
}

const glm::vec3& Camera::getViewUp() const
{
	return mViewUp;
}

float Camera::getFov() const
{
	return mFovAngle;
}

const glm::mat4& Camera::getModelMatrix()
{
	if (mViewMatrixDirty)
	{
		computeViewMatrix();
		mViewMatrixDirty = false;
	}
	return mModelMatrix;
}

void Camera::setPosition(const glm::vec3& pos)
{
	setPosition(pos.x, pos.y, pos.z);
}

void Camera::setPosition(float pos[3])
{
	setPosition(pos[0], pos[1], pos[2]);
}

void Camera::setPosition(float x, float y, float z)
{
	if (mPosition.x != x || mPosition.y != y || mPosition.z != z)
	{
		mPosition.x = x;
		mPosition.y = y;
		mPosition.z = z;
		mViewMatrixDirty = true;
		mViewProjectionMatrixDirty = true;
		computeDistance();
	}
}

void Camera::setFocalPoint(const glm::vec3& center)
{
	setFocalPoint(center.x, center.y, center.z);
}

void Camera::setFocalPoint(float center[3])
{
	setFocalPoint(center[0], center[1], center[2]);
}

void Camera::setFocalPoint(float x, float y, float z)
{
	if (mCenter.x != x || mCenter.y != y || mCenter.z != z)
	{
		mCenter.x = x;
		mCenter.y = y;
		mCenter.z = z;
		computeDistance();
		mViewMatrixDirty = true;
		mViewProjectionMatrix = true;
	}
}

void Camera::setViewUp(const glm::vec3& up)
{
	if (mViewUp != up)
	{
		float norm = glm::length(up);
		if (norm != 0)
		{
			mViewUp = glm::normalize(up);
		}
		else
		{
			mViewUp = { 0, 1, 0 };
		}
		mViewMatrixDirty = true;
		mViewProjectionMatrixDirty = true;
	}
}

void Camera::computeDistance()
{
	auto viewDir = this->mCenter - mPosition;
	this->mDistance = glm::length(viewDir);
	if (this->mDistance < mMinDistance)
	{
		this->mDistance = mMinDistance;
		this->mCenter = this->mPosition + mViewDirection * mDistance;
	}
	this->mViewDirection = glm::normalize(viewDir);
	computeViewPlaneNormal();
}

void Camera::computeViewPlaneNormal()
{
	mViewPlaneNormal = mViewDirection * -1.0f;
}

void Camera::setFovAngle(float angle)
{
	if (mFovAngle != angle)
	{
		mFovAngle = angle < mMinFovAngle ? 
			mMinFovAngle : angle > mMaxFovAngle ? 
			mMaxFovAngle : angle;
		mProjectionMatrixDirty = true;
		mViewProjectionMatrixDirty = true;
	}
}

void Camera::setAspectRatio(int width, int height)
{
	float ratio = static_cast<float>(width) / static_cast<float>(height);
	mViewport.x = width;
	mViewport.y = height;
	if (mAspectRatio != ratio)
	{
		mAspectRatio = ratio;
		mProjectionMatrixDirty = true;
		mViewProjectionMatrixDirty = true;
	}
}

void Camera::setUseOrthographic(bool enable)
{
	if (mIsOrthographic != enable)
	{
		mIsOrthographic = enable;
		mProjectionMatrixDirty = true;
		mViewProjectionMatrixDirty = true;
	}
}

void Camera::setParallelScale(float scale)
{
	if (mParallelScale != scale)
	{
		mParallelScale = scale;
		mProjectionMatrixDirty = true;
		mViewProjectionMatrixDirty = true;
	}
}

void Camera::computeViewMatrix()
{
	mViewMatrix = glm::lookAt(mPosition, mCenter, mViewUp);
	mModelMatrix = glm::inverse(mViewMatrix);
	mViewMatrixDirty = false;
}

void Camera::computeProjectionMatrix()
{
	if (mIsOrthographic)
	{
		float height = mParallelScale;
		float width = height * mAspectRatio;
		mProjectionMatrix = glm::ortho(
			-width, width, 
			-height, height, 
			mClippingRange[0], mClippingRange[1]
		);
	}
	else
	{
		mProjectionMatrix = glm::perspective(glm::radians(mFovAngle), mAspectRatio, mClippingRange[0], mClippingRange[1]);
	}
	mProjectionMatrixDirty = false;
}

