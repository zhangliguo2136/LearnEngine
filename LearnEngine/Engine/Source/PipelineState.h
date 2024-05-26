#pragma once
#include <string>
#include <d3d12.h>
#include <unordered_map>
#include <wrl/module.h>
#include "Shader.h"

class TGraphicsPSODescriptor
{
public:
	TGraphicsPSODescriptor();

	bool operator==(const TGraphicsPSODescriptor& Other) const //TODO
	{
		return Other.InputLayoutName == InputLayoutName
			&& Other.Shader == Shader
			&& Other.PrimitiveTopologyType == PrimitiveTopologyType;
	}

public:
	static D3D12_BLEND_DESC DefaultBlend();
	static D3D12_RASTERIZER_DESC DefaultRasterizer();
	static D3D12_DEPTH_STENCIL_DESC DefaultDepthStencil();
	static DXGI_SAMPLE_DESC DefaultDXGISample();

public:
	std::string InputLayoutName;
	TShader* Shader = nullptr;

	UINT NumRenderTargets = 0;
	DXGI_FORMAT RTVFormats[8] = { DXGI_FORMAT_UNKNOWN };
	DXGI_FORMAT DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	DXGI_SAMPLE_DESC SampleDesc;
	D3D12_RASTERIZER_DESC RasterizerDesc;
	D3D12_BLEND_DESC BlendDesc;
	D3D12_DEPTH_STENCIL_DESC DepthStencilDesc;

	D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

// declare hash<TGraphicsPSODescriptor>
namespace std
{
	template <>
	struct hash<TGraphicsPSODescriptor>
	{
		std::size_t operator()(const TGraphicsPSODescriptor& Descriptor) const
		{
			using std::hash;
			using std::string;

			// Compute individual hash values for each item,
			// and combine them using XOR
			// and bit shifting:
			return (hash<string>()(Descriptor.InputLayoutName)
				^ (hash<void*>()(Descriptor.Shader) << 1));
		}
	};
}

class TD3DRHI;
class TInputLayoutManager;
class TGraphicsPSOManager
{
public:
	TGraphicsPSOManager(TD3DRHI* InD3DRHI, TInputLayoutManager* InInputLayoutManager);

	ID3D12PipelineState* FindAndCreate(const TGraphicsPSODescriptor& Descriptor);
	void TryCreate(const TGraphicsPSODescriptor& Descriptor);
private:
	TD3DRHI* D3DRHI = nullptr;

	TInputLayoutManager* InputLayoutManager = nullptr;

	std::unordered_map<TGraphicsPSODescriptor, Microsoft::WRL::ComPtr<ID3D12PipelineState>> PSOMap;
};