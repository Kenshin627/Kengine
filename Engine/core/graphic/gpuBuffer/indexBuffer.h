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
	/// 索引数据类型
	/// </summary>
	uint mDataType;
	/// <summary>
	/// 渲染的顶点个数
	/// </summary>
	uint mCount;
};