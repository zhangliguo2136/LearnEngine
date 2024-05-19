#include "Material.h"

TMaterial::TMaterial(const std::string& InName, const std::string& InShaderName)
	:Name(InName), ShaderName(InShaderName)
{
}

TShader* TMaterial::GetShaderByDefines(const TMacroDefines& MacroDefines, TD3DRHI* D3DRHI)
{
	auto Iter = ShaderMap.find(MacroDefines);
	if (Iter == ShaderMap.end())
	{
		// Create new shader By Defines
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = ShaderName;
		ShaderInfo.FileName = ShaderName;
		ShaderInfo.ShaderDefines = MacroDefines;
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		std::unique_ptr<TShader> NewShader = std::make_unique<TShader>(ShaderInfo, D3DRHI);

		ShaderMap.insert({ MacroDefines, std::move(NewShader) });

		return ShaderMap[MacroDefines].get();
	}
	else
	{
		return Iter->second.get();
	}
}
