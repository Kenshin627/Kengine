#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include "core.h"
#include "shader.h"


Shader::Shader(ShaderType type, const char* source)
	:mRendererID(0),
	 mShaderType(type)
{

	mRendererID = glCreateShader(toGLShaderType(type));
	GLCALL(glShaderSource(mRendererID, 1, &source, nullptr));
	GLCALL(glCompileShader(mRendererID));
	int success;
	GLCALL(glGetShaderiv(mRendererID, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		std::vector<char> errorLog;
		int logLength;
		GLCALL(glGetShaderiv(mRendererID, GL_INFO_LOG_LENGTH, &logLength));
		errorLog.reserve(logLength);
		GLCALL(glGetShaderInfoLog(mRendererID, logLength, 0, errorLog.data()));
		KS_CORE_ERROR("[SHADER COMPILE FAIED]:");
		KS_CORE_WARN("[SHADER TYPE]: {0}", static_cast<uint>(mShaderType));
		KS_CORE_WARN("[ERROR]: {0}", errorLog.data());
	}
}

Ref<Shader> Shader::fromSource(ShaderType type, const char* source)
{
	return std::make_shared<Shader>(type, source);
}

Ref<Shader> Shader::fromFile(ShaderType type, const char* path)
{
	const std::string& source = Shader::readFile(path);
	return std::make_shared<Shader>(type, source.c_str());
}

std::string Shader::readFile(const char* path)
{
	std::ifstream file;
	file.open(path, std::ios_base::binary);
	if(!file.is_open())
	{
		KS_CORE_ERROR("[FILE NOT FOUND]: {0}", path);
		return "";

	}
	std::stringstream ss;
	ss << file.rdbuf();
	if (file.fail())
	{
		KS_CORE_ERROR("[FILE READ ERROR]: {0}", path);
		return "";
	}
	return ss.str();
}

Shader::~Shader()
{
	GLCALL(glDeleteShader(mRendererID));
}

uint Shader::id() const
{
	return mRendererID;
}

ShaderType Shader::type() const
{
	return mShaderType;
}

uint Shader::toGLShaderType(ShaderType type)
{
	switch (type)
	{
		case ShaderType::Vertex:			return GL_VERTEX_SHADER;
		case ShaderType::Fragment:			return GL_FRAGMENT_SHADER;
		case ShaderType::Geometry:			return GL_GEOMETRY_SHADER;
		case ShaderType::Compute:			return GL_COMPUTE_SHADER;
		case ShaderType::TessControl:		return GL_TESS_CONTROL_SHADER;
		case ShaderType::TessEvaluation:	return GL_TESS_EVALUATION_SHADER;
		default:							return 0;
	}
}
