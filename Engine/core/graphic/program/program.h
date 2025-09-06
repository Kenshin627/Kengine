#pragma once
#include <glm.hpp>
#include <unordered_map>
#include "typedef.h"
#include "graphic/const.h"
#include "shader.h"

struct ShaderFile
{
	const char* path;
	ShaderType type;
};

struct ShaderSource
{
	const char* source;
	ShaderType type;
};

class Program
{
public:
	Program();
	~Program();
	Program(const Program& program) = delete;
	Program(const Program&& program) = delete;
	Program& operator=(const Program& pro) = delete;
	void buildFromSources(const std::initializer_list<ShaderSource>& sources);
	void buildFromFiles(const std::initializer_list<ShaderFile>& files);
	void attachShader(Ref<Shader> shader);	
	void link();
	void bind() const;
	void unBind() const;
	int  getUniformLocation(const char* name) const;	
	void setUniform(const char* name, int value) const;
	void setUniform(const char* name, float value) const;
	void setUniform(const char* name, const glm::vec2& value) const;
	void setUniform(const char* name, const glm::vec3& value) const;
	void setUniform(const char* name, const glm::vec4& value) const;
	void setUniform(const char* name, const glm::mat3& value, bool transpose = false) const;
	void setUniform(const char* name, const glm::mat4& value, bool transpose = false) const;
	uint id() const;
	void checkLinkState() const;
private:
	uint										  mRendererID;
	bool										  mLinked;
	mutable std::unordered_map<const char*, uint> mUniformLocationCache;
	//std::vector<Ref<Shader>>					  mShaders;
};