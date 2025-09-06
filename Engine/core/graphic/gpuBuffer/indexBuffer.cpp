#include "core.h"
#include "indexBuffer.h"

IndexBuffer::IndexBuffer()
	:Buffer()
{
}

IndexBuffer::IndexBuffer(const void* data, uint size, uint dataType, uint bufferUsage)
	:Buffer(size, data, bufferUsage),
	 mDataType(dataType),
	 mCount(size / getDataTypeSize(dataType))
{
}

IndexBuffer::~IndexBuffer()	{ }

void IndexBuffer::bind() const
{
	GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id()));
}

void IndexBuffer::unBind() const
{
	GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

uint IndexBuffer::getDataTypeSize(uint dataType)
{
	switch (dataType)
	{
	case GL_UNSIGNED_BYTE:
		return sizeof(GLubyte);
	case GL_UNSIGNED_SHORT:
		return sizeof(GLushort);
	case GL_UNSIGNED_INT:
		return sizeof(GLuint);
	default:
		return 0;
	}
}
