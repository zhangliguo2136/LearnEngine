#include "D3DResource.h"

TD3DResource::TD3DResource()
{

}

TD3DResource::TD3DResource(Microsoft::WRL::ComPtr<ID3D12Resource> InD3DResource, D3D12_RESOURCE_STATES InState)
	:D3DResource(InD3DResource), CurrentState(InState)
{
}

TD3DResource::~TD3DResource()
{
}

TD3DResourceInitInfo TD3DResourceInitInfo::Buffer(uint32_t Size)
{
	TD3DResourceInitInfo InitInfo;
	InitInfo.ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	InitInfo.ResourceDesc.Width = Size;
	InitInfo.ResourceDesc.Height = 1;
	InitInfo.ResourceDesc.MipLevels = 1;
	InitInfo.ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	InitInfo.ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	return InitInfo;
}

TD3DResourceInitInfo TD3DResourceInitInfo::Texture2D(uint32_t Width, uint32_t Height, DXGI_FORMAT Format)
{
	TD3DResourceInitInfo InitInfo;
	InitInfo.ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	InitInfo.ResourceDesc.Width = Width;
	InitInfo.ResourceDesc.Height = Height;
	InitInfo.ResourceDesc.Format = Format;
	InitInfo.ResourceDesc.MipLevels = 1;
	InitInfo.ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	return InitInfo;
}
