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

TD3DResourceInitInfo::TD3DResourceInitInfo()
{
	HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.CreationNodeMask = 1;
	HeapProperties.VisibleNodeMask = 1;

	//HeapDesc.SizeInBytes = DEFAULT_POOL_SIZE;
	//HeapDesc.Properties = HeapProperties;
	//HeapDesc.Alignment = 0;
	//HeapDesc.Flags = D3D12_HEAP_FLAG_NONE;

	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = 0;
	ResourceDesc.Height = 0;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 0;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ClearValue.Format = DXGI_FORMAT_UNKNOWN;
	ClearValue.Color[0] = 0.f;
	ClearValue.Color[1] = 0.f;
	ClearValue.Color[2] = 0.f;
	ClearValue.Color[3] = 0.f;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;
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

TD3DResourceInitInfo TD3DResourceInitInfo::Buffer_Upload(uint32_t Size)
{
	TD3DResourceInitInfo InitInfo = TD3DResourceInitInfo::Buffer(Size);
	InitInfo.InitState = D3D12_RESOURCE_STATE_GENERIC_READ;
	InitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	return InitInfo;
}

TD3DResourceInitInfo TD3DResourceInitInfo::Buffer_Default(uint32_t Size)
{
	TD3DResourceInitInfo InitInfo = TD3DResourceInitInfo::Buffer(Size);
	InitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	return InitInfo;
}

TD3DResourceInitInfo TD3DResourceInitInfo::Texture2D(uint32_t Width, uint32_t Height, DXGI_FORMAT Format)
{
	TD3DResourceInitInfo InitInfo;
	InitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	InitInfo.ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	InitInfo.ResourceDesc.Width = Width;
	InitInfo.ResourceDesc.Height = Height;
	InitInfo.ResourceDesc.Format = Format;
	InitInfo.ResourceDesc.MipLevels = 1;
	InitInfo.ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	InitInfo.ClearValue.Format = Format;

	return InitInfo;
}
