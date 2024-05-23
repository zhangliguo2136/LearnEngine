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
	TD3DResourceInitInfo();

	static TD3DResourceInitInfo Buffer(uint32_t Size);
	static TD3DResourceInitInfo Buffer_Upload(uint32_t Size);
	static TD3DResourceInitInfo Buffer_Default(uint32_t Size);

	static TD3DResourceInitInfo Texture2D(uint32_t Width, uint32_t Height, DXGI_FORMAT Format);
public:
	EAllocationStrategy AllocationStrategy = EAllocationStrategy::StandAlone;

	D3D12_HEAP_PROPERTIES HeapProperties;
	//D3D12_HEAP_DESC HeapDesc;
	D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAG_NONE;

	D3D12_RESOURCE_DESC ResourceDesc;
	D3D12_RESOURCE_STATES InitState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_CLEAR_VALUE ClearValue;
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
