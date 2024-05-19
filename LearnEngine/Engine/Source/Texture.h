#pragma once
#include <string>
#include <vector>

#include "D3DTexture.h"

enum class ETextureType
{
	TEXTURE_2D,
	TEXTURE_CUBE,
	TEXTURE_3D,
};

struct TTextureInfo
{
	int Width;
	int Height;
	int Component;
	int ComponentSize;
	
	int RowBytes;
	int TotalBytes;

	std::vector<uint8_t> Datas;
};

class TTexture
{
public:
	TTexture(const std::string& InName, ETextureType InType, std::string InFilePath)
		:Name(InName), Type(InType), FilePath(InFilePath)
	{

	}

	virtual ~TTexture() {}

	TTexture(const TTexture& Other) = delete;
	TTexture& operator=(const TTexture& Other) = delete;

public:
	std::string Name;
	std::string FilePath;
	ETextureType Type;

	TTextureInfo TextureInfo;
};


class TTexture2D : public TTexture
{
public:
	TTexture2D(const std::string& InName, std::string InFilePath);

	~TTexture2D();
};