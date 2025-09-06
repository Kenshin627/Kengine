#pragma once
#include <glad/glad.h>
#include "typedef.h"

class Buffer
{
public:
	Buffer();
	Buffer(uint size, const void* data = nullptr, uint bufferUsage = GL_DYNAMIC_STORAGE_BIT);
	~Buffer();
	Buffer(const Buffer& buf) = delete;
	Buffer(const Buffer&& buf) = delete;
	Buffer& operator=(const Buffer& buf) = delete;
	void setData(uint size, const void* data, uint offset = 0) const;
	void map(uint length, uint offset = 0, uint usage = 0) const;
	void unMap() const;
	void clearBuffer(uint internalformat, uint size, const void* data, uint format, uint type, uint offset = 0) const;
	void copyBuffer(uint readBuffer, uint writeBuffer, uint readOffset, uint writeOffset, uint size) const;
	virtual void bind() const = 0;
	virtual void unBind() const = 0;
	void allocate(uint size, const void* data = nullptr, uint bufferUsage = GL_DYNAMIC_STORAGE_BIT);
	uint id() const;
	bool isAllocated() const { return mIsAllocated; }
	void checkAllocated() const;
private:
	void createBuffer();
private:
	uint mRendererID;
	uint mSize;
	bool mIsAllocated;
	uint mBufferUsage;
};