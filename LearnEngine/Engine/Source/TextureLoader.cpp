#include "TextureLoader.h"
#include "PNGLoader.h"
#include "JPGLoader.h"
#include "HDRLoader.h"

TTextureLoader::TTextureLoader()
{
	PNGLoader = new TPNGLoader();
	JPGLoader = new TJPGLoader();
	HDRLoader = new THDRLoader();
}

TTextureLoader::~TTextureLoader()
{
}

void TTextureLoader::Load(const std::string& filename, TTextureInfo& TextureInfo)
{
	std::string ext = GetExtension(filename);

	if (ext == "dds")
	{
		LoadDDSTexture(filename, TextureInfo);
	}
	else if (ext == "jpg")
	{
		LoadJPGTexture(filename, TextureInfo);
	}
	else if (ext == "png")
	{
		LoadPNGTexture(filename, TextureInfo);
	}
	else if (ext == "hdr")
	{
		LoadHDRTexture(filename, TextureInfo);
	}
}

std::string TTextureLoader::GetExtension(std::string path)
{
	if (path.rfind('.') != std::string::npos)
	{
		if (path.rfind('.') != (path.length() - 1))
		{
			return path.substr(path.rfind('.') + 1);
		}
	}

	return "";
}

void TTextureLoader::LoadDDSTexture(const std::string& filename, TTextureInfo& TextureInfo)
{
}

void TTextureLoader::LoadHDRTexture(const std::string& filename, TTextureInfo& TextureInfo)
{
	HDRLoader->Load(filename, TextureInfo);
}

void TTextureLoader::LoadPNGTexture(const std::string& filename, TTextureInfo& TextureInfo)
{
	PNGLoader->Load(filename, TextureInfo);
}

void TTextureLoader::LoadJPGTexture(const std::string& filename, TTextureInfo& TextureInfo)
{
	JPGLoader->Load(filename, TextureInfo);
}
