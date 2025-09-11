#include "texture2D/texture2D.h"
#include "textureSystem.h"

TextureSystem::TextureSystem() {}

std::shared_ptr<Texture2D> TextureSystem::getTexture(const std::string& path)
{
	auto iter = mTextures.find(path);
	if (iter != mTextures.cend())
	{
		return iter->second;
	}
	std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>();
	tex->loadFromFile(path.c_str());
	mTextures.insert({ path, tex });
	return tex;
}
