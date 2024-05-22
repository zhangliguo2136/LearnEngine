#pragma once
#include <d3d12.h>
#include <wrl/module.h>
#include <stdint.h>
#define DEFAULT_POOL_SIZE (512 * 1024 * 512)

enum class EAllocationStrategy
{
	StandAlone,
	PlacedResource
};

class TD3DResourceInitInfo
{
public:
	TD3DResourceInitInfo()
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
		ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		//ClearValue.Format = DXGI_FORMAT_UNKNOWN;
		//ClearValue.Color[0] = 0.f;
		//ClearValue.Color[1] = 0.f;
		//ClearValue.Color[2] = 0.f;
		//ClearValue.Color[3] = 0.f;
	}

	static TD3DResourceInitInfo Buffer(uint32_t Size);
	static TD3DResourceInitInfo Texture2D(uint32_t Width, uint32_t Height, DXGI_FORMAT Format);
public:
	EAllocationStrategy AllocationStrategy = EAllocationStrategy::StandAlone;

	D3D12_HEAP_PROPERTIES HeapProperties;
	//D3D12_HEAP_DESC HeapDesc;
	D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAG_NONE;

	D3D12_RESOURCE_DESC ResourceDesc;
	D3D12_RESOURCE_STATES InitState = D3D12_RESOURCE_STATE_COMMON;

	//D3D12_CLEAR_VALUE ClearValue;
};

class TD3DResource
{
public:
	TD3DResource();
	TD3DResource(Microsoft::WRL::ComPtr<ID3D12Resource>, D3D12_RESOURCE_STATES);
	~TD3DResource();

	Microsoft::WRL::ComPtr<ID3D12Resource> D3DResource = nullptr;
	D3D12_RESOURCE_STATES CurrentState = D3D12_RESOURCE_STATE_COMMON;

	ID3D12Heap* BackingHeap = nullptr;
	uint64_t OffsetFromBaseOfHeap = 0;
};
