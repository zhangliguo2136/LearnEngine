#pragma once
#include <string>
#include <unordered_map>
#include <wrl/module.h>

#include "d3d12.h"
#include "D3DView.h"
#include "D3DResource.h"

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
	TD3DResource* ConstantBuffer;
};

struct TShaderSRVParameter : TShaderParameter
{
	UINT BindCount;
	std::vector<TD3DShaderResourceView*> SRVList;
};

struct TShaderUAVParameter : TShaderParameter
{
	UINT BindCount;
	std::vector<TD3DUnorderedAccessView*> UAVList;
};

struct TShaderSamplerParameter : TShaderParameter
{

};

class TD3DRHI;
class TD3DShaderResourceView;
class TShader
{
public:
	TShader(const TShaderInfo& InShaderInfo, TD3DRHI* InRHI);
	void Initialize();

	bool SetParameter(std::string ParamName, TD3DResource* CBuffer);

	bool SetParameter(std::string ParamName, TD3DShaderResourceView* SRV);
	bool SetParameter(std::string ParamName, const std::vector<TD3DShaderResourceView*>& SRVList);

	bool SetParameter(std::string ParamName, TD3DUnorderedAccessView* UAV);
	bool SetParameter(std::string ParamName, const std::vector<TD3DUnorderedAccessView*>& UAVList);

	void BindParameters();
private:
	static Microsoft::WRL::ComPtr<ID3DBlob> Compile(const std::string& Filename, const D3D_SHADER_MACRO* Defines, const std::string& Entrypoint, const std::string& Target);

	void GetShaderParameters(Microsoft::WRL::ComPtr<ID3DBlob> PassBlob, EShaderType ShaderType);

	D3D12_SHADER_VISIBILITY GetShaderVisibility(EShaderType ShaderType);

	std::vector<D3D12_STATIC_SAMPLER_DESC> CreateStaticSamplers();

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