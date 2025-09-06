#include <glad/glad.h>
#include "uniformBuffer.h"
#include "core.h"

UniformBuffer::UniformBuffer()
	:mBindingPoint(0),
	 Buffer()
{
}

UniformBuffer::UniformBuffer(uint size, uint binding, const void* data, uint bufferUsage)
	:Buffer(size, data, bufferUsage),
	 mBindingPoint(binding)
{
}

void UniformBuffer::bindingToPoint(uint binding)
{

	mBindingPoint = binding;
	GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, binding, id()));
}

void UniformBuffer::bind() const
{
	GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, mBindingPoint, id()));
}

void UniformBuffer::unBind() const
{
	GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, mBindingPoint, 0));
}

uint UniformBuffer::getBindingPoint() const
{
	return mBindingPoint;
}
