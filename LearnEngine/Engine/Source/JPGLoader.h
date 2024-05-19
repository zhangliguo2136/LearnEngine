#pragma once

#include <string>

#include "Texture.h"

class TJPGLoader
{
public:
	TJPGLoader() {}
	~TJPGLoader() {};

	void Load(const std::string& filename, TTextureInfo& TextureInfo);
};