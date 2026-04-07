#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>
#include "cameraController.h"
#include "camera.h"
#include <logger.h>
#include <GLFW/glfw3.h>

static const float PI = 3.14159265358979f;
static const float TWOPI = PI * 2.0;
static const float EPS = 0.000001;

CameraController::CameraController(Camera* camera)
    :mCamera(camera)
{
    const glm::vec3& up = camera->getViewUp();
    mQuat = glm::rotation(up, glm::vec3(0, 1, 0));
    mQuatInverse = glm::inverse(mQuat);
    mRestorePosition = camera->getPosition();
    //TODO
    //mRestoraZoom =
    mRestoreTarget = camera->getCenter();
}

float CameraController::getPolarAngle() const
{
    return mSpherical.mPhi;
}

float CameraController::getAzimuthalAngle() const
{
    return mSpherical.mTheta;
}

float CameraController::getDistance() const
{
    return glm::distance(mCamera->getPosition(), mCamera->getCenter());
}

void CameraController::saveState()
{
    //TODO
    mRestorePosition = mCamera->getPosition();
    mRestoreTarget = mCamera->getCenter();
    //TODO
    //mRestoraZoom
}

void CameraController::reset()
{
    //TODO
}

void CameraController::pan(int deltaX, int deltaY)
{
    _pan(deltaX, deltaY);
    update();
}

void CameraController::dollyIn(float dollyScale)
{
    _dollyIn(dollyScale);
    update();
}

void CameraController::dollyOut(float dollyScale)
{
    _dollyOut(dollyScale);
    update();
}

void CameraController::rotateLeft(float angle)
{
    rotationLeft(angle);
    update();
}

void CameraController::rotateUp(float angle)
{
    rotationUp(angle);
    update();
}

void CameraController::update()
{
    //TODO
    const glm::vec3& position = mCamera->getPosition();
    glm::vec3 target = mCamera->getCenter();
    glm::vec3 v = position - target;
    v = glm::rotate(mQuat, v);
    mSpherical.setFromVector3(v);

    if (mAutoRotate && mState == CameraState::None)
    {
        //TODO
        rotationLeft(getAutoRotationAngle(0.0));
    }

    if (mEnableDamping)
    {
        mSpherical.mTheta += mSphericalDelta.mTheta * mDampingFactor;
        mSpherical.mPhi += mSphericalDelta.mPhi * mDampingFactor;
    }
    else
    {
        mSpherical.mTheta += mSphericalDelta.mTheta;
        mSpherical.mPhi += mSphericalDelta.mPhi;
    }

    float minAzimuthalAngle = mMinAzimuthalAngle;
    float maxAzimuthalAngle = mMaxAzimuthalAngle;

    if (isfinite(minAzimuthalAngle) && isfinite(maxAzimuthalAngle))
    {
        if (minAzimuthalAngle < -PI) minAzimuthalAngle += TWOPI; else if (minAzimuthalAngle > PI) minAzimuthalAngle -= TWOPI;
        if (maxAzimuthalAngle < -PI) maxAzimuthalAngle += TWOPI; else if (maxAzimuthalAngle > PI) maxAzimuthalAngle -= TWOPI;

        if (minAzimuthalAngle <= maxAzimuthalAngle)
        {
            mSpherical.mTheta = glm::max(minAzimuthalAngle, glm::min(maxAzimuthalAngle, mSpherical.mTheta));
        }
        else
        {
            mSpherical.mTheta = (mSpherical.mTheta > (minAzimuthalAngle + maxAzimuthalAngle) / 2) ?
                glm::max(minAzimuthalAngle, mSpherical.mTheta) :
                glm::min(maxAzimuthalAngle, mSpherical.mTheta);
        }
    }

    mSpherical.mPhi = glm::max(mMinPolarAngle, glm::min(mMaxPolarAngle, mSpherical.mPhi));
    mSpherical.makeSafe();

    if (mEnableDamping)
    {
        target += mPanOffset * mDampingFactor;
    }
    else
    {
        target += mPanOffset;
    }
    mCamera->setFocalPoint(target);

    //zoom
    bool zoomChanged;
    float prevRadius = mSpherical.mRadius;
    mSpherical.mRadius = clampDistance(prevRadius * mScale);
    zoomChanged = prevRadius != mSpherical.mRadius;

    v = setFromSphericalCoords(mSpherical.mRadius, mSpherical.mPhi, mSpherical.mTheta);

    v = glm::rotate(mQuatInverse, v);

    glm::vec3 newPos = mCamera->getCenter() + v;
    mCamera->setPosition(newPos);

    if (mEnableDamping) 
    {
        mSphericalDelta.mTheta *= (1 - mDampingFactor);
        mSphericalDelta.mPhi *= (1 - mDampingFactor);
        mPanOffset *= (1 - mDampingFactor);
    }
    else 
    {
        mSphericalDelta = { 0, 0, 0 };
        mPanOffset = { 0, 0, 0 };
    }
    mScale = 1;
}

double CameraController::getAutoRotationAngle(double deltaTime)
{
    //TODO check deltaTime
    return TWOPI / 60 / 60 * mAutoRotateSpeed;
}

float CameraController::getZoomScale(float delta)
{
    float normalizeDelta = abs(delta * 0.01);
    return pow(0.95, mZoomSpeed * normalizeDelta);
}

void CameraController::rotationLeft(float angle)
{
    mSphericalDelta.mTheta -= angle;
}

void CameraController::rotationUp(float angle)
{
    mSphericalDelta.mPhi -= angle;
}

void CameraController::panLeft(float distance)
{
    const glm::mat4& cameraModelMatrix = mCamera->getModelMatrix();
    glm::vec3 xAxis = cameraModelMatrix[0];
    xAxis *= -distance;
    mPanOffset += xAxis;
}

void CameraController::panUp(float distance)
{
    const glm::mat4& cameraModelMatrix = mCamera->getModelMatrix();
    glm::vec3 axis;
    if (mScreenSpacePanning)
    {
        axis = cameraModelMatrix[1];
    }
    else
    {
        axis = cameraModelMatrix[0];
        axis = glm::cross(mCamera->getViewUp(), axis);
    }

    axis *= distance;
    mPanOffset += axis;
}

void CameraController::_pan(float deltaX, float deltaY)
{
    const glm::vec3& position = mCamera->getPosition();
    const glm::vec3& target = mCamera->getCenter();
    float fov = mCamera->getFov(); //angle
    const glm::vec3 v = position - target;
    float distance = glm::length(v);
    float height = mCamera->getViewport().y;
    distance *= tan(glm::radians(fov / 2.0f));
    float factor = 2.0f * distance / height;
    panLeft(deltaX * factor);
    panUp(deltaY * factor);
}

void CameraController::_dollyOut(float dollyScale)
{
    mScale /= dollyScale;
}

void CameraController::_dollyIn(float dollyScale)
{
    mScale *= dollyScale;
}

void CameraController::_updateZoomParameters(float x, float y)
{
    //TODO
}

float CameraController::clampDistance(float dist)
{
    return glm::max(mMinDistance, glm::min(mMaxDistance, dist));
}

glm::vec3 CameraController::setFromSphericalCoords(float radius, float phi, float theta)
{
    float sinPhiRadius = sin(phi) * radius;
    glm::vec3 result;
    result.x = sinPhiRadius * sin(theta);
    result.y = cos(phi) * radius;
    result.z = sinPhiRadius * cos(theta);
    return result;
}

/// <summary>
/// Events
/// </summary>
/// <param name="button"></param>
/// <param name="action"></param>
/// <param name="mods"></param>
void CameraController::onMouseButton(int button, int action, int mods, double x, double y)
{
    KS_CORE_TRACE("Mouse button: {}, action: {}, mods: {}", button, action, mods);
    if (action == GLFW_PRESS)
    {
        switch (button)
        {
            //left
        case 0:
            if (!mEnableRotate)
            {
                return;
            }
            _handleMouseDownRotate(x, y);
            mState = CameraState::Rotate;
            break;
        case 1:
            if (!mEnableZoom)
            {
                return;
            }
            _handleMouseDownZoom(x, y);
            mState = CameraState::Dolly;
            break;

        case 2:
            if (!mEnablePan)
            {
                return;
            }
            _handleMouseDownPan(x, y);
            mState = CameraState::Pan;
            break;

        default:
            return;
        }
    }
    else
    {
        mState = CameraState::None;
    }
}

void CameraController::onMouseMove(double xpos, double ypos)
{
    KS_CORE_TRACE("Mouse move: {}, {}", xpos, ypos);

    switch (mState) 
    {

        case CameraState::Rotate:
            if (!mEnableRotate)
            {
                return;
            }
            _handleMouseMoveRotate(xpos, ypos);
            break;

        case CameraState::Dolly:
            if (!mEnableZoom)
            {
                return;
            }
            _handleMouseMoveDolly(xpos, ypos);
            break;

        case CameraState::Pan:
            if (!mEnablePan)
            {
                return;
            }
            _handleMouseMovePan(xpos, ypos);
            break;
    }
}

void CameraController::onScroll(double xoffset, double yoffset)
{
    KS_CORE_TRACE("Mouse scroll: {}, {}", xoffset, yoffset);
	if (!mEnableZoom)
	{
		return;
	}
	if (yoffset < 0)
	{
		_dollyOut(getZoomScale(yoffset) * 1.3);
	}
	else if (yoffset > 0)
	{
		_dollyIn(getZoomScale(yoffset) * 1.3);
	}
	update();
}

void CameraController::_handleMouseDownRotate(double x, double y)
{
	mRotateStart.x = x;
	mRotateStart.y = y;
}

void CameraController::_handleMouseDownZoom(double x, double y)
{
    mDollyStart.x = x;
    mDollyStart.y = y;
}

void CameraController::_handleMouseDownPan(double x, double y)
{
    mPanStart.x = x;
    mPanStart.y = y;
}

void CameraController::_handleMouseMoveRotate(double x, double y)
{
    mRotateEnd.x = x;
    mRotateEnd.y = y;

    mRotateDelta = mRotateEnd - mRotateStart;
    mRotateDelta *= mRotateSpeed;

	rotationLeft(TWOPI * mRotateDelta.x / mCamera->getViewport().y);
	rotationUp(TWOPI * mRotateDelta.y / mCamera->getViewport().y);
	mRotateStart = mRotateEnd;
    update();
}

void CameraController::_handleMouseMoveDolly(double x, double y)
{
    mDollyEnd = { x, y };

    mDollyDelta = mDollyEnd - mDollyStart;
    if (mDollyDelta.y > 0)
    {
        _dollyOut(getZoomScale(mDollyDelta.y));
    }
    else if (mDollyDelta.y < 0)
    {
        _dollyIn(getZoomScale(mDollyDelta.y));
    }

    mDollyStart = mDollyEnd;
    update();
}

void CameraController::_handleMouseMovePan(double x, double y)
{
	mPanEnd.x = x;
	mPanEnd.y = y;
	mPanDelta = mPanEnd - mPanStart;
	mPanDelta *= mPanSpeed;
	_pan(mPanDelta.x, mPanDelta.y);
	mPanStart = mPanEnd;
	update();
}
