#include "Shader.h"
#include "D3DRHI.h"
#include "Utils.h"
#include "FileSystem.h"
#include "FormatConvert.h"
#include "D3DDevice.h"
#include "D3DCommandContent.h"

#include <d3dcompiler.h>
#include <assert.h>

void TMacroDefines::GetD3DShaderMacro(std::vector<D3D_SHADER_MACRO>& OutMacros) const
{
	for (const auto& Pair : DefinesMap)
	{
		D3D_SHADER_MACRO Macro;
		Macro.Name = Pair.first.c_str();
		Macro.Definition = Pair.second.c_str();
		OutMacros.push_back(Macro);
	}

	D3D_SHADER_MACRO Macro;
	Macro.Name = NULL;
	Macro.Definition = NULL;
	OutMacros.push_back(Macro);
}

bool TMacroDefines::operator==(const TMacroDefines& Other) const
{
	if (DefinesMap.size() != Other.DefinesMap.size())
	{
		return false;
	}

	for (const auto& Pair : DefinesMap)
	{
		const std::string Key = Pair.first;
		const std::string Value = Pair.second;

		auto Iter = Other.DefinesMap.find(Key);
		if (Iter == Other.DefinesMap.end() || Iter->second != Value)
		{
			return false;
		}
	}

	return true;
}

void TMacroDefines::SetDefine(const std::string& Name, const std::string Definition)
{
	DefinesMap.insert_or_assign(Name, Definition);
}

TShader::TShader(const TShaderInfo& InShaderInfo, TD3DRHI* InD3DRHI)
	: ShaderInfo(InShaderInfo), D3DRHI(InD3DRHI)
{
	this->Initialize();
}

void TShader::Initialize()
{
	std::string ShaderDir = TFileSystem::EngineDir() + "Resource/Shaders/";
	std::string FilePath = ShaderDir + ShaderInfo.FileName + ".hlsl";

	std::vector<D3D_SHADER_MACRO> ShaderMacros;
	ShaderInfo.ShaderDefines.GetD3DShaderMacro(ShaderMacros);

	if (ShaderInfo.bCreateVS)
	{
		auto VSBlob = Compile(FilePath, ShaderMacros.data(), ShaderInfo.VSEntryPoint, "vs_5_1");
		ShaderPass["VS"] = VSBlob;

		GetShaderParameters(VSBlob, EShaderType::VERTEX_SHADER);
	}

	if (ShaderInfo.bCreatePS)
	{
		auto PSBlob = Compile(FilePath, ShaderMacros.data(), ShaderInfo.PSEntryPoint, "ps_5_1");
		ShaderPass["PS"] = PSBlob;

		GetShaderParameters(PSBlob, EShaderType::PIXEL_SHADER);
	}

	if (ShaderInfo.bCreateCS)
	{
		auto CSBlob = Compile(FilePath, ShaderMacros.data(), ShaderInfo.CSEntryPoint, "cs_5_1");
		ShaderPass["CS"] = CSBlob;

		GetShaderParameters(CSBlob, EShaderType::COMPUTE_SHADER);
	}

	// Create rootSignature
	CreateRootSignature();
}

bool TShader::SetParameter(std::string ParamName, TD3DResource* CBuffer)
{
	bool FindParam = false;

	for (TShaderCBVParameter& Param : CBVParams)
	{
		if (Param.Name == ParamName)
		{
			Param.ConstantBuffer = CBuffer;

			FindParam = true;
		}
	}

	return FindParam;
}

bool TShader::SetParameter(std::string ParamName, TD3DShaderResourceView* SRV)
{
	std::vector<TD3DShaderResourceView*> SRVList;
	SRVList.push_back(SRV);

	return SetParameter(ParamName, SRVList);
}

bool TShader::SetParameter(std::string ParamName, const std::vector<TD3DShaderResourceView*>& SRVList)
{
	bool FindParam = false;

	for (TShaderSRVParameter& Param : SRVParams)
	{
		if (Param.Name == ParamName)
		{
			Param.SRVList = SRVList;
			FindParam = true;
		}
	}

	return FindParam;
}

bool TShader::SetParameter(std::string ParamName, TD3DUnorderedAccessView* UAV)
{
	std::vector<TD3DUnorderedAccessView*> UAVList;
	UAVList.push_back(UAV);

	return SetParameter(ParamName, UAVList);
}

bool TShader::SetParameter(std::string ParamName, const std::vector<TD3DUnorderedAccessView*>& UAVList)
{
	bool FindParam = false;

	for (TShaderUAVParameter& Param : UAVParams)
	{
		if (Param.Name == ParamName)
		{
			Param.UAVList = UAVList;
			FindParam = true;
		}
	}

	return FindParam;
}

void TShader::BindParameters()
{
	// CBV Binding
	{
		for (int i = 0; i < CBVParams.size(); i++)
		{
			UINT RootParamIdx = CBVSignatureBaseBindSlot + i;
			D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress = CBVParams[i].ConstantBuffer->D3DResource->GetGPUVirtualAddress();
			D3DRHI->GetCommandContent()->GetCommandList()->SetGraphicsRootConstantBufferView(RootParamIdx, GPUVirtualAddress);
		}
	}

	//SRV Binding
	{
		if (SRVCount > 0)
		{
			std::vector<TD3DDescriptor> SrcDescriptors;
			SrcDescriptors.resize(SRVCount);

			for (const TShaderSRVParameter& Param : SRVParams)
			{
				for (UINT i = 0; i < Param.SRVList.size(); i++)
				{
					UINT Index = Param.BindPoint + i;
					SrcDescriptors[Index] = Param.SRVList[i]->GetDescriptor();
				}
			}

			UINT RootParamIdx = SRVSignatureBindSlot;

			TD3DDescriptor OutDescriptor;
			D3DRHI->GetCommandContent()->GetDescriptorCache()->AppendDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SrcDescriptors, OutDescriptor);
			D3DRHI->GetCommandContent()->GetCommandList()->SetGraphicsRootDescriptorTable(RootParamIdx, OutDescriptor.GpuHandle);
		}
	}

	// UAV Binding
	{
		//TODO With Compute Shader
	}

	// Clear
	{
		for (TShaderCBVParameter& Param : CBVParams)
		{
			Param.ConstantBuffer = nullptr;
		}

		for (TShaderSRVParameter& Param : SRVParams)
		{
			Param.SRVList.clear();
		}

		for (TShaderUAVParameter& Param : UAVParams)
		{
			Param.UAVList.clear();
		}
	}
}

Microsoft::WRL::ComPtr<ID3DBlob> TShader::Compile(const std::string& Filename, const D3D_SHADER_MACRO* Defines, const std::string& Entrypoint, const std::string& Target)
{
	UINT CompileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable better shader debugging with the graphics debugging tools.
	CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<ID3DBlob> ByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> Errors;
	hr = D3DCompileFromFile(
		TFormatConvert::StrToWStr(Filename).c_str(), 
		Defines, 
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		Entrypoint.c_str(), 
		Target.c_str(), 
		CompileFlags, 
		0, 
		&ByteCode, 
		&Errors
	);

	if (Errors != nullptr)
	{
		OutputDebugStringA((char*)Errors->GetBufferPointer());
	}

	ThrowIfFailed(hr);

	return ByteCode;
}

void TShader::GetShaderParameters(Microsoft::WRL::ComPtr<ID3DBlob> PassBlob, EShaderType ShaderType)
{
	ID3D12ShaderReflection* Reflection = NULL;
	D3DReflect(PassBlob->GetBufferPointer(), PassBlob->GetBufferSize(), IID_ID3D12ShaderReflection, (void**)&Reflection);

	D3D12_SHADER_DESC ShaderDesc;
	Reflection->GetDesc(&ShaderDesc);

	//printf("ShaderName: %s \n", ShaderInfo.ShaderName.c_str());

	for (UINT i = 0; i < ShaderDesc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC  ResourceDesc;
		Reflection->GetResourceBindingDesc(i, &ResourceDesc);

		auto ShaderVarName = ResourceDesc.Name;
		auto ResourceType = ResourceDesc.Type;
		auto RegisterSpace = ResourceDesc.Space;
		auto BindPoint = ResourceDesc.BindPoint;
		auto BindCount = ResourceDesc.BindCount;

		//printf("ShaderVarName: %s, ", ShaderVarName);
		//printf("ResourceType: %d, ", ResourceType);
		//printf("RegisterSpace: %d \n", RegisterSpace);
		//printf("BindPoint:  %d, ", BindPoint);
		//printf("BindCount: %d \n", BindCount);

		if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER)
		{
			TShaderCBVParameter Param;
			Param.Name = ShaderVarName;
			Param.ShaderType = ShaderType;
			Param.BindPoint = BindPoint;
			Param.RegisterSpace = RegisterSpace;

			CBVParams.push_back(Param);
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED
			|| ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE)
		{
			TShaderSRVParameter Param;
			Param.Name = ShaderVarName;
			Param.ShaderType = ShaderType;
			Param.BindPoint = BindPoint;
			Param.BindCount = BindCount;
			Param.RegisterSpace = RegisterSpace;

			SRVParams.push_back(Param);
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED
			|| ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED)
		{
			assert(ShaderType == EShaderType::COMPUTE_SHADER);

			TShaderUAVParameter Param;
			Param.Name = ShaderVarName;
			Param.ShaderType = ShaderType;
			Param.BindPoint = BindPoint;
			Param.BindCount = BindCount;
			Param.RegisterSpace = RegisterSpace;

			UAVParams.push_back(Param);
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER)
		{
			assert(ShaderType == EShaderType::PIXEL_SHADER);

			TShaderSamplerParameter Param;
			Param.Name = ShaderVarName;
			Param.ShaderType = ShaderType;
			Param.BindPoint = BindPoint;
			Param.RegisterSpace = RegisterSpace;

			SamplerParams.push_back(Param);
		}
	}
}

D3D12_SHADER_VISIBILITY TShader::GetShaderVisibility(EShaderType ShaderType)
{
	D3D12_SHADER_VISIBILITY ShaderVisibility;
	if (ShaderType == EShaderType::VERTEX_SHADER)
	{
		ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	}
	else if (ShaderType == EShaderType::PIXEL_SHADER)
	{
		ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}
	else if (ShaderType == EShaderType::COMPUTE_SHADER)
	{
		ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	else
	{
		assert(0);
	}

	return ShaderVisibility;
}

std::vector<D3D12_STATIC_SAMPLER_DESC> TShader::CreateStaticSamplers()
{
	D3D12_STATIC_SAMPLER_DESC PointWrap;
	PointWrap.ShaderRegister = 0;
	PointWrap.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	PointWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	PointWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	PointWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	PointWrap.MipLODBias = 0;
	PointWrap.MaxAnisotropy = 16;
	PointWrap.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	PointWrap.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	PointWrap.MinLOD = 0.f;
	PointWrap.MaxLOD = D3D12_FLOAT32_MAX;
	PointWrap.RegisterSpace = 0;
	PointWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC PointClamp;
	PointClamp.ShaderRegister = 1;
	PointClamp.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	PointClamp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	PointClamp.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	PointClamp.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	PointClamp.MipLODBias = 0;
	PointClamp.MaxAnisotropy = 16;
	PointClamp.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	PointClamp.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	PointClamp.MinLOD = 0.f;
	PointClamp.MaxLOD = D3D12_FLOAT32_MAX;
	PointClamp.RegisterSpace = 0;
	PointClamp.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC LinearWrap;
	LinearWrap.ShaderRegister = 2;
	LinearWrap.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	LinearWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	LinearWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	LinearWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	LinearWrap.MipLODBias = 0;
	LinearWrap.MaxAnisotropy = 16;
	LinearWrap.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	LinearWrap.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	LinearWrap.MinLOD = 0.f;
	LinearWrap.MaxLOD = D3D12_FLOAT32_MAX;
	LinearWrap.RegisterSpace = 0;
	LinearWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC LinearClamp;
	LinearClamp.ShaderRegister = 3;
	LinearClamp.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	LinearClamp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	LinearClamp.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	LinearClamp.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	LinearClamp.MipLODBias = 0;
	LinearClamp.MaxAnisotropy = 16;
	LinearClamp.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	LinearClamp.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	LinearClamp.MinLOD = 0.f;
	LinearClamp.MaxLOD = D3D12_FLOAT32_MAX;
	LinearClamp.RegisterSpace = 0;
	LinearClamp.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_STATIC_SAMPLER_DESC AnisotropicWrap;
	AnisotropicWrap.ShaderRegister = 4;
	AnisotropicWrap.Filter = D3D12_FILTER_ANISOTROPIC;
	AnisotropicWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	AnisotropicWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	AnisotropicWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	AnisotropicWrap.MipLODBias = 0.f;
	AnisotropicWrap.MaxAnisotropy = 8;
	AnisotropicWrap.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	AnisotropicWrap.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	AnisotropicWrap.MinLOD = 0.f;
	AnisotropicWrap.MaxLOD = D3D12_FLOAT32_MAX;
	AnisotropicWrap.RegisterSpace = 0;
	AnisotropicWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC AnisotropicClamp;
	AnisotropicClamp.ShaderRegister = 5;
	AnisotropicClamp.Filter = D3D12_FILTER_ANISOTROPIC;
	AnisotropicClamp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	AnisotropicClamp.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	AnisotropicClamp.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	AnisotropicClamp.MipLODBias = 0.f;
	AnisotropicClamp.MaxAnisotropy = 8;
	AnisotropicClamp.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	AnisotropicClamp.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	AnisotropicClamp.MinLOD = 0.f;
	AnisotropicClamp.MaxLOD = D3D12_FLOAT32_MAX;
	AnisotropicClamp.RegisterSpace = 0;
	AnisotropicClamp.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_STATIC_SAMPLER_DESC Shadow;
	Shadow.ShaderRegister = 6;
	Shadow.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	Shadow.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	Shadow.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	Shadow.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	Shadow.MipLODBias = 0.f;
	Shadow.MaxAnisotropy = 16;
	Shadow.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	Shadow.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	Shadow.MinLOD = 0.f;
	Shadow.MaxLOD = D3D12_FLOAT32_MAX;
	Shadow.RegisterSpace = 0;
	Shadow.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_STATIC_SAMPLER_DESC DepthMap;
	DepthMap.ShaderRegister = 7;
	DepthMap.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	DepthMap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	DepthMap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	DepthMap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	DepthMap.MipLODBias = 0.f;
	DepthMap.MaxAnisotropy = 0;
	DepthMap.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DepthMap.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	DepthMap.MinLOD = 0.f;
	DepthMap.MaxLOD = D3D12_FLOAT32_MAX;
	DepthMap.RegisterSpace = 0;
	DepthMap.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	std::vector<D3D12_STATIC_SAMPLER_DESC> StaticSamplers;
	StaticSamplers.push_back(PointWrap);
	StaticSamplers.push_back(PointClamp);
	StaticSamplers.push_back(LinearWrap);
	StaticSamplers.push_back(LinearClamp);
	StaticSamplers.push_back(AnisotropicWrap);
	StaticSamplers.push_back(AnisotropicClamp);
	StaticSamplers.push_back(Shadow);
	StaticSamplers.push_back(DepthMap);

	return StaticSamplers;
}

void TShader::CreateRootSignature()
{
	//------------------------------------------------Set SlotRootParameter---------------------------------------
	std::vector<D3D12_ROOT_PARAMETER> SlotRootParameter;

	// CBV
	for (const TShaderCBVParameter& Param : CBVParams)
	{
		if (CBVSignatureBaseBindSlot == -1)
		{
			CBVSignatureBaseBindSlot = (UINT)SlotRootParameter.size();
		}


		D3D12_ROOT_PARAMETER RootParam;
		RootParam.ShaderVisibility = GetShaderVisibility(Param.ShaderType);
		RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		RootParam.Descriptor.ShaderRegister = Param.BindPoint;
		RootParam.Descriptor.RegisterSpace = Param.RegisterSpace;

		SlotRootParameter.push_back(RootParam);

	}

	// SRV
	{
		for (const TShaderSRVParameter& Param : SRVParams)
		{
			SRVCount += Param.BindCount;
		}

		if (SRVCount > 0)
		{
			SRVSignatureBindSlot = (UINT)SlotRootParameter.size();

			D3D12_DESCRIPTOR_RANGE SRVTable;
			SRVTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			SRVTable.NumDescriptors = SRVCount;
			SRVTable.BaseShaderRegister = 0;
			SRVTable.RegisterSpace = 0;
			SRVTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER RootParam;
			RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			RootParam.ShaderVisibility = ShaderInfo.bCreateCS ? D3D12_SHADER_VISIBILITY_ALL : D3D12_SHADER_VISIBILITY_PIXEL;
			RootParam.DescriptorTable.NumDescriptorRanges = 1;
			RootParam.DescriptorTable.pDescriptorRanges = &SRVTable;

			SlotRootParameter.push_back(RootParam);
		}
	}

	// UAV
	{
		for (const TShaderUAVParameter& Param : UAVParams)
		{
			UAVCount += Param.BindCount;
		}

		if (UAVCount > 0)
		{
			UAVSignatureBindSlot = (UINT)SlotRootParameter.size();

			D3D12_DESCRIPTOR_RANGE UAVTable;
			UAVTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			UAVTable.NumDescriptors = UAVCount;
			UAVTable.BaseShaderRegister = 0;
			UAVTable.RegisterSpace = 0;
			UAVTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER RootParam;
			RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			RootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			RootParam.DescriptorTable.NumDescriptorRanges = 1;
			RootParam.DescriptorTable.pDescriptorRanges = &UAVTable;

			SlotRootParameter.push_back(RootParam);
		}
	}

	// Sampler
	// TODO
	auto StaticSamplers = CreateStaticSamplers();

	//------------------------------------------------SerializeRootSignature---------------------------------------
	D3D12_ROOT_SIGNATURE_DESC RootSigDesc;
	RootSigDesc.NumParameters = (UINT)SlotRootParameter.size();
	RootSigDesc.pParameters = SlotRootParameter.data();
	RootSigDesc.NumStaticSamplers = (UINT)StaticSamplers.size();
	RootSigDesc.pStaticSamplers = StaticSamplers.data();
	RootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(D3DRHI->GetDevice()->GetD3DDevice()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&RootSignature)));
}
