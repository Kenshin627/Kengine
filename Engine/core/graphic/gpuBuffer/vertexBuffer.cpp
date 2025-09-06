#include <glad/glad.h>
#include "vertexBuffer.h"

VertexBuffer::VertexBuffer()
	:Buffer()
{
}

VertexBuffer::VertexBuffer(uint size, const void* data, uint bufferUsage)
	:Buffer(size, data, bufferUsage)
{

}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, id());
}

void VertexBuffer::unBind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
