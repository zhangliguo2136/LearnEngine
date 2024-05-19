#pragma once
#include <d3d12.h>
#include <wrl/module.h>
#include <stdint.h>

enum class EAllocationStrategy
{
	StandAlone,
	PlacedResource
};

struct TD3DResourceInitInfo
{
	EAllocationStrategy AllocationStrategy = EAllocationStrategy::StandAlone;

	D3D12_HEAP_TYPE HeapType;
	D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAG_NONE;
	D3D12_RESOURCE_FLAGS ResourceFlags = D3D12_RESOURCE_FLAG_NONE;

	uint32_t Size = 0;
	uint32_t Alignment = 0;

	D3D12_RESOURCE_STATES InitState;
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
