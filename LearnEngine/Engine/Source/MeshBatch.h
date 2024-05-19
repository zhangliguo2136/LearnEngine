#pragma once
#pragma once
#include <string>
#include "Material.h"
#include <unordered_map>
#include "MeshComponent.h"

struct TMeshBatch
{
	std::string MeshName;

	std::string InputLayoutName;

	TMeshComponent* MeshComponent = nullptr;

	std::shared_ptr<TD3DResource> ObjConstantsRef = nullptr;
	//// Flags
	//bool bUseSDF = false;
};

struct TMeshCommand
{
	struct TMeshShaderParamters
	{
		std::unordered_map<std::string, TD3DResource*> CBVParams;

		std::unordered_map<std::string, std::vector<TD3DShaderResourceView*>> SRVParams;
	};

public:
	void SetShaderParameter(std::string Param, TD3DResource* CBV)
	{
		ShaderParameters.CBVParams.insert(std::make_pair(Param, CBV));
	}

	void SetShaderParameter(std::string Param, TD3DShaderResourceView* SRV)
	{
		std::vector<TD3DShaderResourceView*> SRVs;
		SRVs.push_back(SRV);

		ShaderParameters.SRVParams.insert(std::make_pair(Param, SRVs));
	}

	void SetShaderParameter(std::string Param, std::vector<TD3DShaderResourceView*> SRVs)
	{
		ShaderParameters.SRVParams.insert(std::make_pair(Param, SRVs));
	}

	void ApplyShaderParamters(TShader* Shader) const
	{
		if (Shader)
		{
			for (const auto& Pair : ShaderParameters.CBVParams)
			{
				Shader->SetParameter(Pair.first, Pair.second);
			}

			for (const auto& Pair : ShaderParameters.SRVParams)
			{
				Shader->SetParameter(Pair.first, Pair.second);
			}
		}
	}

public:
	std::string MeshName;

	TMeshShaderParamters ShaderParameters;
};