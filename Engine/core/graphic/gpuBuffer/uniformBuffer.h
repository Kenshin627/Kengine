#pragma once
#include <glm.hpp>
#include "typedef.h"
#include "buffer.h"

class UniformBuffer : public Buffer
{
public:
	UniformBuffer();
	UniformBuffer(uint size, uint binding, const void* data = nullptr, uint bufferUsage = GL_DYNAMIC_STORAGE_BIT);
	~UniformBuffer() = default;
	UniformBuffer(const UniformBuffer& obj) = delete;
	UniformBuffer(UniformBuffer&& obj) = delete;
	UniformBuffer& operator=(const UniformBuffer& obj) = delete;
	void bindingToPoint(uint binding);
	virtual void bind() const override;
	virtual void unBind() const override;
	uint getBindingPoint() const;
private:
	uint mBindingPoint;
};

