#pragma once
#include <string>
#include <unordered_map>
#include <wrl/module.h>

#include "d3dx12.h"

struct TMacroDefines
{
public:
	void GetD3DShaderMacro(std::vector<D3D_SHADER_MACRO>&) const;

	bool operator == (const TMacroDefines& Other) const;

	void SetDefine(const std::string& Name, const std::string Definition);

public:
	std::unordered_map<std::string, std::string> DefinesMap;
};

namespace std
{
	template <>
	struct hash<TMacroDefines>
	{
		std::size_t operator()(const TMacroDefines& Defines) const
		{
			using std::size_t;
			using std::hash;
			using std::string;
			// Compute individual hash values for each string 
			// and combine them using XOR
			// and bit shifting:

			size_t HashValue = 0;
			for (const auto& Pair : Defines.DefinesMap)
			{
				HashValue ^= (hash<string>()(Pair.first) << 1);
				HashValue ^= (hash<string>()(Pair.second) << 1);
			}

			return HashValue;
		}
	};
}

struct TShaderInfo
{
	std::string ShaderName;
	std::string FileName;
	TMacroDefines ShaderDefines;

	bool bCreateVS = false;
	std::string VSEntryPoint = "VS";

	bool bCreatePS = false;
	std::string PSEntryPoint = "PS";

	bool bCreateCS = false;
	std::string CSEntryPoint = "CS";
};

enum class EShaderType
{
	VERTEX_SHADER,
	PIXEL_SHADER,
	COMPUTE_SHADER,
};

struct TShaderParameter
{
	std::string Name;
	EShaderType ShaderType;
	UINT BindPoint;
	UINT RegisterSpace;
};

struct TShaderCBVParameter : TShaderParameter
{
};

struct TShaderSRVParameter : TShaderParameter
{
	UINT BindCount;
};

struct TShaderUAVParameter : TShaderParameter
{
	UINT BindCount;
};

struct TShaderSamplerParameter : TShaderParameter
{

};

class TD3DRHI;
class TShader
{
public:
	TShader(const TShaderInfo& InShaderInfo, TD3DRHI* InRHI);
	void Initialize();

private:
	static Microsoft::WRL::ComPtr<ID3DBlob> Compile(const std::string& Filename, const D3D_SHADER_MACRO* Defines, const std::string& Entrypoint, const std::string& Target);

	void GetShaderParameters(Microsoft::WRL::ComPtr<ID3DBlob> PassBlob, EShaderType ShaderType);

	D3D12_SHADER_VISIBILITY GetShaderVisibility(EShaderType ShaderType);

	std::vector<CD3DX12_STATIC_SAMPLER_DESC> CreateStaticSamplers();

	void CreateRootSignature();

public:
	TShaderInfo ShaderInfo;

	std::vector<TShaderCBVParameter> CBVParams;
	std::vector<TShaderSRVParameter> SRVParams;
	std::vector<TShaderUAVParameter> UAVParams;
	std::vector<TShaderSamplerParameter> SamplerParams;

	int CBVSignatureBaseBindSlot = -1;
	int SRVSignatureBindSlot = -1;
	int UAVSignatureBindSlot = -1;
	int SamplerSignatureBindSlot = -1;

	UINT SRVCount = 0;
	UINT UAVCount = 0;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> ShaderPass;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;

private:
	TD3DRHI* D3DRHI = nullptr;
};