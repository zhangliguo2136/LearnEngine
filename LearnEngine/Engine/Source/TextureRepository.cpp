#include "TextureRepository.h"
#include "FileSystem.h"
#include "TextureLoader.h"

TTextureRepository::TTextureRepository()
{
	TextureLoader = new TTextureLoader();
}

TTextureRepository::~TTextureRepository()
{
	delete TextureLoader;
}

TTextureRepository& TTextureRepository::Get()
{
	static TTextureRepository Instance;
	return Instance;
}

void TTextureRepository::Load()
{
	std::string TextureDir = TFileSystem::EngineDir() + "Resource/Textures/";
	TextureMap.emplace("NullTex", std::make_shared<TTexture2D>("NullTex", TextureDir + "white1x1.jpg"));

	TextureMap.emplace("CyborgWeapon_BaseColor", std::make_shared<TTexture2D>("CyborgWeapon_BaseColor", TextureDir + "CyborgWeapon_BaseColor.png"));
	TextureMap.emplace("CyborgWeapon_Normal", std::make_shared<TTexture2D>("CyborgWeapon_Normal", TextureDir + "CyborgWeapon_Normal.png"));
	TextureMap.emplace("CyborgWeapon_Metallic", std::make_shared<TTexture2D>("CyborgWeapon_Metallic", TextureDir + "CyborgWeapon_Metallic.png"));
	TextureMap.emplace("CyborgWeapon_Roughness", std::make_shared<TTexture2D>("CyborgWeapon_Roughness", TextureDir + "CyborgWeapon_Roughness.png"));

	TextureMap.emplace("Shiodome_Stairs", std::make_shared<TTexture2D>("Shiodome_Stairs", TextureDir + "10-Shiodome_Stairs_3k.hdr"));

	for (auto Pair : TextureMap)
	{
		TextureLoader->Load(Pair.second->FilePath, Pair.second->TextureInfo);
	}
}

void TTextureRepository::Unload()
{
	TextureMap.clear();
}
