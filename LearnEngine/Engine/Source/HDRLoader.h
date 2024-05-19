#pragma once

#include <string>

#include "Texture.h"

class THDRLoader
{
public:
	THDRLoader() {}
	~THDRLoader() {};

	void Load(const std::string& filename, TTextureInfo& TextureInfo);
};