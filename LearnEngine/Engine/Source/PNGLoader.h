#pragma once
#include <string>

#include "Texture.h"

class TPNGLoader
{
public:
	TPNGLoader(){}
	~TPNGLoader() {};

	void Load(const std::string& filename, TTextureInfo& TextureInfo);
};