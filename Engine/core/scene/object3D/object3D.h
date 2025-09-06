#pragma once
#include <memory>
#include <gtc/matrix_transform.hpp>
#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include "eventSystem/eventDispacher.h"

class Object3D : public EventDispatcher
{
public:
	Object3D() = default;
	virtual ~Object3D() = default;
	Object3D(const Object3D& obj) = delete;
	Object3D(Object3D&& obj) = delete;
	Object3D& operator=(const Object3D& obj) = delete;
private:
	std::string mName;
	std::weak_ptr<Object3D> mParent;
	std::vector<std::weak_ptr<Object3D>> mChildren;
	glm::vec3 mPosition   { 0,0,0      };
	glm::vec3 mScale	  { 1,1,1      };
	glm::quat mRotation   { 1, 0, 0, 0 };
	glm::mat4 mLocalMatrix{ 1.0f };
	glm::mat4 mWorldMatrix{ 1.0f };

};