#pragma once
#include "Texture.h"

class TPNGLoader;
class TJPGLoader;
class THDRLoader;
class TTextureLoader
{
public:
	TTextureLoader();
	~TTextureLoader();

	void Load(const std::string& filename, TTextureInfo& TextureInfo);

private:
	std::string GetExtension(std::string path);

	void LoadDDSTexture(const std::string& filename, TTextureInfo& TextureInfo);
	void LoadHDRTexture(const std::string& filename, TTextureInfo& TextureInfo);
	void LoadPNGTexture(const std::string& filename, TTextureInfo& TextureInfo);
	void LoadJPGTexture(const std::string& filename, TTextureInfo& TextureInfo);

private:
	TPNGLoader* PNGLoader = nullptr;
	TJPGLoader* JPGLoader = nullptr;
	THDRLoader* HDRLoader = nullptr;
};