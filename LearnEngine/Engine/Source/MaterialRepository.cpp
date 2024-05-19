#include "MaterialRepository.h"

TMaterialRepository& TMaterialRepository::Get()
{
	static TMaterialRepository Instance;
	return Instance;
}

void TMaterialRepository::Load()
{
	{
		TMaterial* DefaultMat = CreateMaterial("DefaultMat", "BasePassDefault");
		DefaultMat->TextureMap.emplace("BaseColorTexture", "NullTex");
		DefaultMat->TextureMap.emplace("NormalTexture", "NullTex");
		DefaultMat->TextureMap.emplace("MetallicTexture", "NullTex");
		DefaultMat->TextureMap.emplace("RoughnessTexture", "NullTex");

		CreateMaterialInstance(DefaultMat, DefaultMat->Name + "_Inst");

		{
			{
				TMaterialInstance* CyborgWeaponMatInst = CreateMaterialInstance(DefaultMat, "CyborgWeaponMat_Inst");
				CyborgWeaponMatInst->TextureMap.emplace("BaseColorTexture", "CyborgWeapon_BaseColor");
				CyborgWeaponMatInst->TextureMap.emplace("NormalTexture", "CyborgWeapon_Normal");
				CyborgWeaponMatInst->TextureMap.emplace("MetallicTexture", "CyborgWeapon_Metallic");
				CyborgWeaponMatInst->TextureMap.emplace("RoughnessTexture", "CyborgWeapon_Roughness");
			}
		}
	}

	{
		// Material
		TMaterial* SkyMat = CreateMaterial("SkyMat", "BasePassSky");
		SkyMat->TextureMap.emplace("SkyCubeTexture", "Shiodome_Stairs");

		SkyMat->ShadingModel = EShadingMode::Unlit;

		//TMaterialRenderState& RenderState = SkyMat->RenderState;
		//RenderState.CullMode = D3D12_CULL_MODE_NONE;
		//RenderState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		// MaterialInstance
		CreateMaterialInstance(SkyMat, SkyMat->Name + "_Inst");
	}
}

void TMaterialRepository::Unload()
{
	MaterialMap.clear();
	MaterialInstanceMap.clear();
}

TMaterialInstance* TMaterialRepository::GetMaterialInstance(const std::string& MaterialInstanceName) const
{
	TMaterialInstance* Result = nullptr;

	auto Iter = MaterialInstanceMap.find(MaterialInstanceName);
	if (Iter != MaterialInstanceMap.end())
	{
		Result = Iter->second.get();
	}

	return Result;
}

TMaterial* TMaterialRepository::CreateMaterial(const std::string& MaterialName, const std::string& ShaderName)
{
	MaterialMap.insert({ MaterialName, std::make_unique<TMaterial>(MaterialName, ShaderName) });

	return MaterialMap[MaterialName].get();
}

TMaterialInstance* TMaterialRepository::CreateMaterialInstance(TMaterial* Material, const std::string& MaterialInstanceName)
{
	MaterialInstanceMap.insert({ MaterialInstanceName, std::make_unique<TMaterialInstance>(Material, MaterialInstanceName) });

	return MaterialInstanceMap[MaterialInstanceName].get();
}

