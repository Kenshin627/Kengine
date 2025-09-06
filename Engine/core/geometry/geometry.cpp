#include "geometry.h"
#include <core.h>

Geometry::Geometry()
{
	//buildGeometry();
}

void Geometry::draw() const
{
	if (mVAO)
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
		mVAO->unBind();
	}
}
