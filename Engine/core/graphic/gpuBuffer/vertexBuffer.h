#pragma once
#include "buffer.h"

class VertexBuffer :public Buffer
{
public:
	VertexBuffer();
	VertexBuffer(uint size, const void* data = nullptr, uint bufferUsage = GL_DYNAMIC_STORAGE_BIT);
	~VertexBuffer();
	virtual void bind() const override;
	virtual void unBind() const override;
};
