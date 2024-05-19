#pragma once
#include "D3DTexture.h"
#include "Texture.h"

class TD3DRHI;
class TD3DTextureLoader
{
public:
	TD3DTextureLoader(TD3DRHI* InD3DRHI);
	~TD3DTextureLoader();

	void Load(TD3DTextureRef TextureRef, TTextureInfo& TextureInfo );

private:
	TD3DRHI* D3DRHI = nullptr;
};