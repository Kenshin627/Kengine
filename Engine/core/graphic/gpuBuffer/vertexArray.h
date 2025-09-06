#pragma once
#include <glad/glad.h>
#include <vector>
#include "typedef.h"

class VertexBuffer;
class IndexBuffer;
struct AttributeLayout
{
	AttributeLayout() {}
	AttributeLayout(uint attrIndex, uint buffer, uint bindingPoint, uint offset, uint stride, uint components, uint type, bool normalized = false, uint relativeOffset = 0, uint d = 0)
		:attributeIndex(attrIndex),
		bufferId(buffer),
		bindingPoint(bindingPoint),
		offset(offset),
		stride(stride),
		components(components),
		type(type),
		normalized(normalized),
		relativeOffset(relativeOffset),
		divisor(d)
	{
	}
	uint		 attributeIndex		{ 0 };
	uint		 bufferId			{ 0 };
	uint		 bindingPoint		{ 0 };
	uint		 offset				{ 0 };
	uint		 stride				{ 0 };
	uint		 components			{ 0 };
	uint		 type				{ 0 };
	bool         normalized			{ 0 };
	uint		 relativeOffset		{ 0 };
	uint		 divisor			{ 0 };
};

class VertexArray
{
public:
	VertexArray(uint primitiveCount, uint pritimiveType = 0x0004);
	~VertexArray();
	void bind() const;
	void unBind() const;
	void addAttribute(const AttributeLayout& layout);
	void addAttributes(const std::initializer_list<AttributeLayout>& layouts);
	void buildIndexBuffer(const void* data, uint size, uint dataType, uint bufferUsage);
	uint buildVertexBuffer(uint size, const void* data = nullptr, uint bufferUsage = 0);
	uint id() const;
	uint getPrimitiveType() const;
	uint getPrimitiveCount() const;
	bool isIndexedDraw() const;
	uint getIndexBufferType() const;
private:
	uint							 mRendererID;
	uint							 mPrimitiveType;
	uint							 mPrimitiveCount;
	uint							 mIndexBufferType;
	Scope<IndexBuffer>				 mIndexBuffer;
	bool							 mIndexedDraw;
	std::vector<Ref<VertexBuffer>>   mVBOs;
};