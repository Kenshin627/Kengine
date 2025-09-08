#include <vector>
#include <gtc/type_ptr.hpp>
#include "core.h"
#include "program.h"
#pragma once

Program::Program()
	:mRendererID(0),
	 mLinked(false)
{
	mRendererID = GLCALLRES(glCreateProgram());
}

Program::~Program()
{
	GLCALL(glDeleteProgram(mRendererID));
}

void Program::buildFromSources(const std::initializer_list<ShaderSource>& sources)
{	
	for (auto& shaderSource : sources)
	{
		attachShader(Shader::fromSource(shaderSource.type, shaderSource.source));
	}
	link();
}

void Program::buildFromFiles(const std::initializer_list<ShaderFile>& files)
{
	for (auto& shaderFile : files)
	{
		attachShader(Shader::fromFile(shaderFile.type, shaderFile.path));
	}
	link();
}

void Program::attachShader(Ref<Shader> shader)
{
	GLCALL(glAttachShader(mRendererID, shader->id()));
}

void Program::link()
{
	mUniformLocationCache.clear();
	GLCALL(glLinkProgram(mRendererID));
	int success;
	GLCALL(glGetProgramiv(mRendererID, GL_LINK_STATUS, &success));
	if (!success)
	{
		int logLength = 0;
		GLCALL(glGetProgramiv(mRendererID, GL_INFO_LOG_LENGTH, &logLength));
		std::vector<char> errors;
		errors.reserve(logLength);
		GLCALL(glGetProgramInfoLog(mRendererID, logLength, nullptr, errors.data()));
		KS_CORE_ERROR("[PROGRAM LINK FAIED]:");
		KS_CORE_WARN("[ERROR]: {0}", errors.data());
		mLinked = false;
		return;
	}
	mLinked = true;
}

void Program::bind() const
{
	//checkLinkState();
	GLCALL(glUseProgram(mRendererID));
}

void Program::unBind() const
{
	//checkLinkState();
	GLCALL(glUseProgram(0));
}

int Program::getUniformLocation(const char* name) const
{
	//checkLinkState();
	auto iter = mUniformLocationCache.find(name);
	if (iter != mUniformLocationCache.end())
	{
		return iter->second;
	}
	int loc = glGetUniformLocation(mRendererID, name);
	if (loc != -1)
	{
		mUniformLocationCache.insert({ name, loc });
		return loc;
	}
	//else
	//{
	//	KS_CORE_WARN("[UNIFORM NOT FOUND]: {0}", name);
	//}
	return -1;
}

void Program::setUniform(const char* name, int value) const
{
	//checkLinkState();
	int loc = getUniformLocation(name);
	if (loc != -1)
	{

		GLCALL(glUniform1i(loc, value));
	}
}

void Program::setUniform(const char* name, uint value) const
{
	int loc = getUniformLocation(name);
	if (loc != -1)
	{

		GLCALL(glUniform1ui(loc, value));
	}
}

void Program::setUniform(const char* name, float value) const
{
	//checkLinkState();
	int loc = getUniformLocation(name);
	if (loc != -1)
	{

		GLCALL(glUniform1f(loc, value));
	}
}

void Program::setUniform(const char* name, const glm::vec2& value) const
{
	//checkLinkState();
	int loc = getUniformLocation(name);
	if (loc != -1)
	{

		GLCALL(glUniform2f(loc, value.x, value.y));
	}
}

void Program::setUniform(const char* name, const glm::vec3& value) const
{
	//checkLinkState();
	int loc = getUniformLocation(name);
	if (loc != -1)
	{

		GLCALL(glUniform3f(loc, value.x, value.y, value.z));
	}
}

void Program::setUniform(const char* name, const glm::vec4& value) const
{
	//checkLinkState();
	int loc = getUniformLocation(name);
	if (loc != -1)
	{

		GLCALL(glUniform4f(loc, value.x, value.y, value.z, value.w));
	}
}

void Program::setUniform(const char* name, const glm::mat3& value, bool transpose) const
{
	//checkLinkState();
	int loc = getUniformLocation(name);
	if (loc != -1)
	{

		GLCALL(glUniformMatrix3fv(getUniformLocation(name), 1, transpose, glm::value_ptr(value)));
	}
}

void Program::setUniform(const char* name, const glm::mat4& value, bool transpose) const
{
	//checkLinkState();
	int loc = getUniformLocation(name);
	if (loc != -1)
	{

		GLCALL(glProgramUniformMatrix4fv(mRendererID, getUniformLocation(name), 1, transpose, glm::value_ptr(value)));
	}
}

uint Program::id() const
{
	return mRendererID;
}

void Program::checkLinkState() const
{
	if (!mLinked)
	{
		KS_CORE_WARN("[PROGRAM HAS NOT LINKED ALREADY]");
		return;
	}
}
