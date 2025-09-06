#pragma once
#include <glad/glad.h>
#include "logger.h"

#define KS_CORE_ASSERT(x, ...) { if(!(x)) { KS_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define KS_CORE_ASSERT_NO_BREAK(x, ...) { if(!(x)) { KS_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); } }

#define GLCALL(x)						  \
	GLClearError();					      \
    x;									  \
    gLCheckError(#x, __FILE__, __LINE__); \

#define GLCALLRES(x)				      \
	([&]() {							  \
	GLClearError();					      \
    auto result = x;					  \
    gLCheckError(#x, __FILE__, __LINE__); \
	return result;						  \
	}())								  \

/* 清除错误 */
static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

//将错误代码转换为字符串描述
static const char* glErrorToString(GLenum error) {
	switch (error) {
	case GL_NO_ERROR:          return "No error";
	case GL_INVALID_ENUM:      return "Invalid enum (无效枚举值)";
	case GL_INVALID_VALUE:     return "Invalid value (无效数值)";
	case GL_INVALID_OPERATION: return "Invalid operation (无效操作)";
	case GL_STACK_OVERFLOW:    return "Stack overflow (栈溢出)";
	case GL_STACK_UNDERFLOW:   return "Stack underflow (栈下溢)";
	case GL_OUT_OF_MEMORY:     return "Out of memory (内存不足)";
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer operation (无效帧缓冲操作)";
#endif
#ifdef GL_CONTEXT_LOST
	case GL_CONTEXT_LOST:      return "Context lost (上下文丢失)";
#endif
	default:                   return "Unknown error (未知错误)";
	}
}

static void gLCheckError(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError() != GL_NO_ERROR)
	{
		KS_CORE_ERROR("[Opengl Error] {0}:\r\n {1}\r\n {2}\r\n {3}\r\n", glErrorToString(error), function, file, line);
	}
}

