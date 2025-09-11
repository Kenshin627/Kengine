#pragma once
#include<unordered_map>
#include<string>
#include<memory>

class Texture2D;
class TextureSystem
{
public:
	~TextureSystem() = default;
	TextureSystem(const TextureSystem& t) = delete;
	TextureSystem(const TextureSystem&& t) = delete;
	TextureSystem& operator=(const TextureSystem& t) = delete;
	TextureSystem& operator=(const TextureSystem&& t) = delete;
	static TextureSystem& getInstance()
	{
		static TextureSystem t;
		return t;
	}
	std::shared_ptr<Texture2D> getTexture(const std::string& path);
private:
	TextureSystem();
private:
	std::unordered_map<std::string, std::shared_ptr<Texture2D>> mTextures;
};