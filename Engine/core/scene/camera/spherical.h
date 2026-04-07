#pragma once
#include <glm.hpp>

class Spherical
{
public:
    Spherical(float radius = 1.0f, float phi = 0.0f, float theta = 0.0f);
    ~Spherical() {};
    void set(float radius, float phi, float theta);
    void makeSafe();
    void setFromVector3(const glm::vec3& v);
    void setFromCartesianCoords(float x, float y, float z);
public:
    float mRadius;
    float mPhi;
    float mTheta;
};