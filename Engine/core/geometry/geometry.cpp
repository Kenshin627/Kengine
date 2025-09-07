#include "geometry.h"
#include <core.h>

Geometry::Geometry()
{
}

void Geometry::beginDraw() const
{
	if (!mVAO)
	{
		KS_CORE_ERROR("no vao is initialized!");
	}
	mVAO->bind();
}

void Geometry::endDraw()
{
	if (!mVAO)
	{
		KS_CORE_ERROR("no vao is initialized!");
	}
	mVAO->unBind();
}

void Geometry::draw() const
{
	mVAO->bind();
	if (mVAO->isIndexedDraw())
	{
		GLCALL(glDrawElements(mVAO->getPrimitiveType(), mVAO->getPrimitiveCount(), mVAO->getIndexBufferType(), (void*)0));
	}
	else
	{
		GLCALL(glDrawArrays(mVAO->getPrimitiveType(), 0, mVAO->getPrimitiveCount()));
	}
}

