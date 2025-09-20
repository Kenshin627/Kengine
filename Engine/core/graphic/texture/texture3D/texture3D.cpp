#include <glad/glad.h>
#include <gtc/type_ptr.hpp>
#include "core.h"
#include "texture3D.h"

Texture3D::Texture3D(const TextureSpecification& spec)
	:Texture(spec),
	 mDepth(spec.depth)
{
	GLCALL(glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &mRendererID));
	GLCALL(glTextureStorage3D(mRendererID, mMipmapLevels, convertToGLInternalFormat(mInternalFormat), mWidth, mHeight, mDepth));
	setWarpModeS(mWrapS);
	setWarpModeT(mWrapT);
	setMinFilter(mMinFilter);
	setMagFilter(mMagFilter);
	if (spec.warpS == TextureWarpMode::CLAMP_TO_BORDER || spec.warpT == TextureWarpMode::CLAMP_TO_BORDER)
	{
		glTextureParameterfv(mRendererID, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(spec.mBorderColor));
	}
}
