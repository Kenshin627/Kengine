#pragma once
#include <memory>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <string>

class Geometry;
class Material;
class Scene;
class Program;

enum class RenderObjectType
{
	Mesh,
	Light,
	Camera
};

class RenderObject
{
public:
	RenderObject(const std::string& name);
	RenderObject(const std::string& name, std::shared_ptr<Geometry> mesh, std::shared_ptr<Material> material);
	virtual ~RenderObject() = default;
	RenderObject(const RenderObject& obj) = delete;
	RenderObject(RenderObject&& obj) = delete;
	RenderObject& operator=(const RenderObject& obj) = delete;
	void setMaterial(std::shared_ptr<Material> material);
	void beginDraw(Program* program = nullptr);
	void draw() const;
	void endDraw(Program* p = nullptr);
	void setPosition(const glm::vec3& pos);
	void setPosition(float x, float y, float z);
	const glm::vec3 getPosition() const { return mPosition; }
	void setRotation(const glm::vec3& rotation);
	void setRotation(float val);
	void setRotation(float x, float y, float z);
	void setScale(const glm::vec3& scale);
	void setScale(float x, float y, float z);
	void setScale(float val);
	void setGeometry(std::shared_ptr<Geometry> geometry);
	void updateModelMatrix();
	void setOwner(Scene* s);
	Scene* getOwner();
	std::shared_ptr<Material> getMaterial() const;
	const std::string& getName() const { return mName; };
	RenderObjectType getType() const { return mRenderObjectType; };
protected:
	void setRenderObjectType(RenderObjectType type) { mRenderObjectType = type; }
private:
	std::string				  mName;
	std::shared_ptr<Geometry> mMesh;
	std::shared_ptr<Material> mMaterial;
	glm::vec3				  mPosition                     { 0,0,0 };
	glm::vec3				  mRotation                     { 0,0,0 };
	glm::vec3				  mScale                        { 1,1,1 };
	glm::mat4				  mModelMatrix				  = glm::identity<glm::mat4>();
	glm::mat4				  mModelMatrixInvertTranspose = glm::identity<glm::mat4>();
	Scene*					  mOnwningScene                 { nullptr };
	RenderObjectType		  mRenderObjectType			  = RenderObjectType::Mesh;
};