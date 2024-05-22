#include "MeshRepository.h"

TMeshRepository::TMeshRepository()
{
	FbxLoader = std::make_unique<TFbxLoader>();
	FbxLoader->Init();
}

TMeshRepository& TMeshRepository::Get()
{
	static TMeshRepository Instance;
	return Instance;
}

void TMeshRepository::Load()
{
	TMesh SphereMesh;
	SphereMesh.CreateSphere(0.5f, 20, 20);
	SphereMesh.MeshName = "SphereMesh";
	MeshMap.emplace("SphereMesh", std::move(SphereMesh));

	TMesh ScreenQuadMesh;
	ScreenQuadMesh.CreateQuad(-1.0f, 1.0f, 2.0f, 2.0f, 0.0f);
	ScreenQuadMesh.MeshName = "ScreenQuadMesh";
	MeshMap.emplace("ScreenQuadMesh", std::move(ScreenQuadMesh));

	TMesh CyborgWeaponMesh;
	CyborgWeaponMesh.MeshName = "CyborgWeapon";
	FbxLoader->LoadFBXMesh("Cyborg_Weapon.fbx", CyborgWeaponMesh);
	MeshMap.emplace("CyborgWeapon", std::move(CyborgWeaponMesh));
}

void TMeshRepository::Unload()
{
	MeshMap.clear();
}
