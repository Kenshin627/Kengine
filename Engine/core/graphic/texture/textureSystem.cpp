#include "texture2D/texture2D.h"
#include "textureSystem.h"

TextureSystem::TextureSystem() {}

std::shared_ptr<Texture2D> TextureSystem::getTexture(const std::string& path, bool flipY, bool SRGB)
{
	auto iter = mTextures.find(path);
	if (iter != mTextures.cend())
	{
		return iter->second;
	}
	TextureSpecification spec;
	spec.mipmapLevel = 5;
	spec.minFilter = TextureFilter::LINEAR_MIPMAP_LINEAR;
	std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>(spec);
	tex->loadFromFile(path.c_str(), flipY, SRGB);
	mTextures.insert({ path, tex });
	return tex;
}
