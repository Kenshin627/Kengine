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

/* ������� */
static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

//���������ת��Ϊ�ַ�������
static const char* glErrorToString(GLenum error) {
	switch (error) {
	case GL_NO_ERROR:          return "No error";
	case GL_INVALID_ENUM:      return "Invalid enum (��Чö��ֵ)";
	case GL_INVALID_VALUE:     return "Invalid value (��Ч��ֵ)";
	case GL_INVALID_OPERATION: return "Invalid operation (��Ч����)";
	case GL_STACK_OVERFLOW:    return "Stack overflow (ջ���)";
	case GL_STACK_UNDERFLOW:   return "Stack underflow (ջ����)";
	case GL_OUT_OF_MEMORY:     return "Out of memory (�ڴ治��)";
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer operation (��Ч֡�������)";
#endif
#ifdef GL_CONTEXT_LOST
	case GL_CONTEXT_LOST:      return "Context lost (�����Ķ�ʧ)";
#endif
	default:                   return "Unknown error (δ֪����)";
	}
}

static void gLCheckError(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError() != GL_NO_ERROR)
	{
		KS_CORE_ERROR("[Opengl Error] {0}:\r\n {1}\r\n {2}\r\n {3}\r\n", glErrorToString(error), function, file, line);
	}
}

