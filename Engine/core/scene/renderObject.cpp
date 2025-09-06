#include "renderObject.h"
#include "material/material.h"
#include "geometry/geometry.h"
#include <gtc/quaternion.hpp>

RenderObject::RenderObject(std::shared_ptr<Geometry> mesh, std::shared_ptr<Material> material)
	:mMesh(std::move(mesh)),
	 mMaterial(material)
{

}

void RenderObject::setMaterial(std::shared_ptr<Material> material)
{
	mMaterial = material;
}

void RenderObject::draw() const
{
	if (mMaterial)
	{
		mMaterial->beginDraw();
		//set model matrix uniform
		mMaterial->getProgram()->setUniform("modelMatrix", mModelMatrix);
		mMaterial->getProgram()->setUniform("modelMatrixInvertTranspose", mModelMatrixInvertTranspose);
		mMesh->draw();
		mMaterial->endDraw();
	}
}

void RenderObject::setPosition(const glm::vec3& pos)
{
	setPosition(pos.x, pos.y, pos.z);
}

void RenderObject::setPosition(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;
	updateModelMatrix();
}

void RenderObject::setRotation(const glm::vec3& rotation)
{
	setRotation(rotation.x, rotation.y, rotation.z);
}

void RenderObject::setRotation(float val)
{
	setRotation(val, val, val);
}

void RenderObject::setRotation(float x, float y, float z)
{
	mRotation.x = x;
	mRotation.y = y;
	mRotation.z = z;
	updateModelMatrix();
}

void RenderObject::setScale(const glm::vec3& scale)
{
	setScale(scale.x, scale.y, scale.z);
}

void RenderObject::setScale(float x, float y, float z)
{
	mScale.x = x;
	mScale.y = y;
	mScale.z = z;
	updateModelMatrix();
}

void RenderObject::setScale(float val)
{
	setScale(val, val, val);
}

void RenderObject::updateModelMatrix()
{
	glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), mPosition);
	glm::quat q = glm::quat(glm::radians(mRotation));
	glm::mat4 rotation = glm::mat4(q);
	glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), mScale);
	mModelMatrix = translation * rotation * scale;
	mModelMatrixInvertTranspose = glm::transpose(glm::inverse(mModelMatrix));
}

void RenderObject::setOwner(Scene* s)
{
	mOnwningScene = s;
}
