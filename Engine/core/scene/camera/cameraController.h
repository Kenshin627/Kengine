#pragma once
#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include "spherical.h"

class Camera;
enum class CameraState
{
    None = -1,
    Rotate = 0,
    Dolly,
    Pan
};

class CameraController
{
public:
    CameraController(Camera* camera);
    float getPolarAngle() const;
    float getAzimuthalAngle() const;
    float getDistance() const;
    void saveState();
    void reset();
    void pan(int deltaX, int deltaY);
    void dollyIn(float dollyScale);
    void dollyOut(float dollyScale);
    void rotateLeft(float angle);
    void rotateUp(float angle);
    void update();
    double getAutoRotationAngle(double deltaTime);
    float getZoomScale(float delta);
public:
    //Events
	void onMouseButton(int button, int action, int mods, double x, double y);
	void onMouseMove(double xpos, double ypos);
	void onScroll(double xoffset, double yoffset);
private:
    void _handleMouseDownRotate(double x, double y);
    void _handleMouseDownZoom(double x, double y);
    void _handleMouseDownPan(double x, double y);
    void _handleMouseMoveRotate(double x, double y);
    void _handleMouseMoveDolly(double x, double y);
    void _handleMouseMovePan(double x, double y);
private:
    void rotationLeft(float angle);
    void rotationUp(float angle);
    void panLeft(float distance);
    void panUp(float distance);
    void _pan(float deltaX, float deltaY);
    void _dollyOut(float dollyScale);
    void _dollyIn(float dollyScale);
    void _updateZoomParameters(float x, float y);
    float clampDistance(float dist);
    glm::vec3 setFromSphericalCoords(float radius, float phi, float theta);

private:
    Camera* mCamera;
    CameraState mState;
    //glm::vec3 mTarget;
    glm::vec3   mCursor;
    float       mMinDistance{0.0f};
    float       mMaxDistance{10000.0f};
    float       mMinTargetRadius;
    float       mMaxTargetRadius;
    float       mMinPolarAngle{ 0 };
    float       mMaxPolarAngle{ 3.14159265358979 };
    float       mMinAzimuthalAngle{ std::numeric_limits<float>::infinity() };
    float       mMaxAzimuthalAngle{ std::numeric_limits<float>::infinity() };
    bool        mEnableDamping{ false };
    float       mDampingFactor{ 0.01f };
    bool        mEnableZoom{ true };
    float       mZoomSpeed{ 1.0f };
    bool        mEnableRotate{ true };
    float       mRotateSpeed{ 0.1f };
    float       mKeyRotateSpeed{ 1.0f };
    bool        mEnablePan{ true };
    float       mPanSpeed{ 1.0f };
    float       mKeyPanSpeed{ 7.0f };
    bool        mScreenSpacePanning{ true };
    bool        mZoomToCursor{ false };
    bool        mAutoRotate{ false };
    float       mAutoRotateSpeed{ 2.0f };

    //save/restore
    glm::vec3   mRestoreTarget;
    glm::vec3   mRestorePosition;
    glm::vec3   mRestoraZoom;

    //cache
    glm::vec3   mLastPosition;
    glm::quat   mLastQuaternion;
    glm::vec3   mLastTargetPosition;

    glm::quat   mQuat;
    glm::quat   mQuatInverse;

    Spherical   mSpherical;
    Spherical   mSphericalDelta;
    float       mScale{ 1.0f };
    glm::vec3   mPanOffset{0.0f};

    glm::vec2   mRotateStart;
    glm::vec2   mRotateEnd;
    glm::vec2   mRotateDelta;

    glm::vec2   mDollyStart;
    glm::vec2   mDollyEnd;
    glm::vec2   mDollyDelta;

    glm::vec2   mPanStart;
    glm::vec2   mPanEnd;
    glm::vec2   mPanDelta;

    glm::vec3   mDollyDirection;
    glm::vec2   mMouse;
    bool        mPerformCursorZoom{ false };
};