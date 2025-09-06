#pragma once
#include <glad/glad.h>
#include "buffer.h"

class IndexBuffer :public Buffer
{
public:
	IndexBuffer();
	IndexBuffer(const void* data, uint size, uint dataType = GL_UNSIGNED_SHORT, uint bufferUsage = GL_DYNAMIC_STORAGE_BIT);
	~IndexBuffer();
	virtual void bind() const override;
	virtual void unBind() const override;
	uint getDataType() const { return mDataType; }
	uint getCount() const { return mCount; }
	static uint getDataTypeSize(uint dataType);
private:
	/// <summary>
	/// ������������
	/// </summary>
	uint mDataType;
	/// <summary>
	/// ��Ⱦ�Ķ������
	/// </summary>
	uint mCount;
};