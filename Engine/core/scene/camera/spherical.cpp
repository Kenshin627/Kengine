#include "spherical.h"

#define PI 3.14159265358979f

Spherical::Spherical(float radius, float phi, float theta)
    :mRadius(radius),
    mPhi(phi),
    mTheta(theta)
{
}

void Spherical::set(float radius, float phi, float theta)
{
    mRadius = radius;
    mPhi = phi;
    mTheta = theta;
}

void Spherical::makeSafe()
{
    float EPS = 0.000001;
    this->mPhi = glm::clamp(mPhi, EPS, PI - EPS);
}

void Spherical::setFromVector3(const glm::vec3& v)
{
    return setFromCartesianCoords(v.x, v.y, v.z);
}

void Spherical::setFromCartesianCoords(float x, float y, float z)
{
    mRadius = std::sqrt(x * x + y * y + z * z);
    if (mRadius == 0)
    {
        mPhi = 0;
        mTheta = 0;
    }
    else
    {
        mPhi = acos(glm::clamp(y / mRadius, -1.0f, 1.0f));
        mTheta = atan2(x, z);
    }
}