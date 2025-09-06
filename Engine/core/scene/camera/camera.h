#pragma once
#include <memory>
#include <glm.hpp>

class Camera
{
public:
	Camera();
	Camera(const glm::vec3& pos, float fov, float aspectRatio, float near = 0.01f, float far= 100.0f);
	~Camera();

	void setPosition(const glm::vec3& pos);
	void setPosition(float pos[3]);
	void setPosition(float x, float y, float z);
	void setFocalPoint(const glm::vec3& point);
	void setFocalPoint(float center[3]);
	void setFocalPoint(float x, float y, float z);
	void setViewUp(const glm::vec3& viewUp);
	void setFovAngle(float angle);
	//void setClippingRange(const glm::vec2& range);
	void setAspectRatio(float ratio);
	void setUseOrthographic(bool enable);
	void setParallelScale(float scale);

	void computeViewMatrix();
	void computeProjectionMatrix();
	void computeDistance();
	void computeViewPlaneNormal();
	
	const glm::vec3&			   getPosition() const;
	const glm::vec3&			   getCenter() const;
	const glm::vec3&			   getViewUp() const;
	const glm::mat4&			   getViewMatrix();
	const glm::mat4&			   getProjectionMatrix();
	const glm::mat4&			   getViewProjectionMatrix();
	bool isViewProjectionDirty() const { return mViewProjectionMatrixDirty; }
	bool isCameraUniformDirty() const { return mViewMatrixDirty || mProjectionMatrixDirty || mViewProjectionMatrixDirty; }
private:
	glm::vec3 mPosition        { 0.0f, 0.0f, 5.0f  };
	glm::vec3 mCenter	       { 0.0f, 0.0f, 0.0f    };
	glm::vec3 mViewUp	       { 0.0f, 1.0f, 0.0f    };
	glm::vec3 mViewDirection   { 0.0f, 0.0f, -1.0f   };
	glm::vec3 mViewPlaneNormal { 0.0f, 0.0f, 1.0f    };

	bool      mIsOrthographic   { false			 };
	float	  mFovAngle         { 30.0f			 };
	float	  mAspectRatio		{ 1.0f			 };
	float     mClippingRange[2] { 0.001f, 100.0f };
	float     mParallelScale    { 1.0f		     };
	float     mThickness;
	float     mDistance;

	glm::mat4 mViewMatrix            { 1.0f  };
	glm::mat4 mProjectionMatrix      { 1.0f  };
	glm::mat4 mViewProjectionMatrix  { 1.0f  };

	bool  mViewMatrixDirty		     { true  };
	bool  mProjectionMatrixDirty	 { true  };
	bool  mViewProjectionMatrixDirty { true  };

	double mMinDistance				 { 1e-20 };
	double mMinFovAngle				 { 0.00000001 };
	double mMaxFovAngle				 { 179.0 };
};