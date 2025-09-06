#include "vertexArray.h"
#include <glad/glad.h>
#include "indexBuffer.h"
#include "vertexBuffer.h"
#include "core.h"

VertexArray::VertexArray(uint primitiveCount, uint pritimiveType)
	:mRendererID(0),
	 mPrimitiveType(pritimiveType),
	 mPrimitiveCount(primitiveCount),
	 mIndexBufferType(0),
	 mIndexedDraw(false)
{
	GLCALL(glCreateVertexArrays(1, &mRendererID));
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &mRendererID);
}

void VertexArray::bind() const
{
	glBindVertexArray(mRendererID);
}

void VertexArray::unBind() const
{
	glBindVertexArray(0);
}

void VertexArray::addAttribute(const AttributeLayout& layout)
{
	GLCALL(glVertexArrayAttribBinding(mRendererID, layout.attributeIndex, layout.bindingPoint));
	GLCALL(glVertexArrayVertexBuffer(mRendererID, layout.bindingPoint, layout.bufferId, layout.offset, layout.stride));
	GLCALL(glVertexArrayAttribFormat(mRendererID, layout.attributeIndex, layout.components, layout.type, layout.normalized, layout.relativeOffset));
	if (layout.divisor > 0)
	{
		GLCALL(glVertexArrayBindingDivisor(mRendererID, layout.bindingPoint, layout.divisor));
	}
	GLCALL(glEnableVertexArrayAttrib(mRendererID, layout.attributeIndex));
}

void VertexArray::addAttributes(const std::initializer_list<AttributeLayout>& layouts)
{
	for (const auto& layout : layouts)
	{
		addAttribute(layout);
	}
}

void VertexArray::buildIndexBuffer(const void* data, uint size, uint dataType, uint bufferUsage)
{
	mIndexBuffer = std::make_unique<IndexBuffer>(data, size, dataType, bufferUsage);
	GLCALL(glVertexArrayElementBuffer(mRendererID, mIndexBuffer->id()));
	mIndexBufferType = dataType;
	mIndexedDraw = true;
}

uint VertexArray::buildVertexBuffer(uint size, const void* data, uint bufferUsage)
{
	auto vbo = std::make_shared<VertexBuffer>(size, data, bufferUsage);
	mVBOs.push_back(vbo);
	return vbo->id();
}

uint VertexArray::id() const
{
	return mRendererID;
}

uint VertexArray::getPrimitiveType() const
{
	return mPrimitiveType;
}

uint VertexArray::getPrimitiveCount() const
{
	return mPrimitiveCount;
}

bool VertexArray::isIndexedDraw() const
{
	return mIndexedDraw;
}

uint VertexArray::getIndexBufferType() const
{
	return mIndexBufferType;
}
