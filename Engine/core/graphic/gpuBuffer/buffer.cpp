#include "core.h"
#include "buffer.h"

Buffer::Buffer()
	:mRendererID(0),
	 mSize(0),
	 mIsAllocated(false)
{
	createBuffer();
}

Buffer::Buffer(uint size, const void* data, uint bufferUsage)
	:mRendererID(0),
	 mSize(size),
	 mIsAllocated(false),
	 mBufferUsage(bufferUsage)
{
	createBuffer();
	allocate(size, data, bufferUsage);
}

Buffer::~Buffer()
{
	GLCALL(glDeleteBuffers(1, &mRendererID));
}

void Buffer::setData(uint size, const void* data, uint offset) const
{
	checkAllocated();
	if (offset + size > mSize)
	{
		KS_CORE_WARN("[BUFFER SET DATA WARNING]: Set data size out of range!");
		return;
	}
	if ((mBufferUsage & GL_DYNAMIC_STORAGE_BIT) == 0)
	{
		KS_CORE_WARN("[BUFFER SET DATA WARNING]: Buffer usage is GL_DYNAMIC_STORAGE_BIT, cannot set data!");
		return;
	}
	GLCALL(glNamedBufferSubData(mRendererID, offset, size, data));
}

void* Buffer::map(uint length, uint offset, uint access) const
{
	checkAllocated();
	void* res = GLCALLRES(glMapNamedBufferRange(mRendererID, offset, length, access));
	return res;
}

void Buffer::unMap() const
{
	checkAllocated();
	GLCALL(glUnmapNamedBuffer(mRendererID));
}

void Buffer::clearBuffer(uint internalformat, uint size, const void* data, uint format, uint type, uint offset) const
{
	checkAllocated();
	//GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data
	GLCALL(glClearNamedBufferSubData(mRendererID, internalformat, offset, size, format, type, data));
}

void Buffer::copyBuffer(uint readBuffer, uint writeBuffer, uint readOffset, uint writeOffset, uint size) const
{
	checkAllocated();
	GLCALL(glCopyBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size));
}

void Buffer::allocate(uint size, const void* data, uint bufferUsage)
{
	if (mIsAllocated)
	{
		KS_CORE_WARN("[BUFFER ALLOCATE WARNING]: Buffer has been allocated!");
		return;
	}
	GLCALL(glNamedBufferStorage(mRendererID, size, data, bufferUsage));
	mBufferUsage = bufferUsage;
	mSize = size;
	mIsAllocated = true;
}

uint Buffer::id() const
{
	return mRendererID;
}

void Buffer::checkAllocated() const
{
	if (!mIsAllocated)
	{
		KS_CORE_ERROR("[BUFFER ERROR]: Buffer has not been allocated!");
	}
}

void Buffer::createBuffer()
{
	GLCALL(glCreateBuffers(1, &mRendererID));
}
