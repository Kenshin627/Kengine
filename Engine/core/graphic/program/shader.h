#pragma once
#include<string>
#include "typedef.h"
#include "graphic/const.h"

class Shader
{
public:
	Shader(ShaderType type, const char* source);
	Shader(const Shader& shader) = delete;
	Shader(const Shader&& shader) = delete;
	Shader& operator=(const Shader& shader) = delete;
	static Ref<Shader> fromSource(ShaderType type, const char* source);
	static Ref<Shader> fromFile(ShaderType type, const char* path);
	//TODO: Load shader from binary
	//static Shader fromBinary(ShaderType type, const char* binary, int length);
	static std::string readFile(const char* path);
	~Shader();
	uint id() const;
	ShaderType type() const;
public:
	static uint toGLShaderType(ShaderType type);
private:
	uint	   mRendererID;
	ShaderType mShaderType;
};