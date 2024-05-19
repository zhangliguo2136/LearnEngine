#pragma once
#include <d3d12.h>
#include <stdint.h>
#include <wrl/module.h>

#include "D3DDevice.h"

class TD3DDescriptorHeap;

class TD3DDescriptor
{
public:
	TD3DDescriptor();
	TD3DDescriptor(SIZE_T CpuHandlePtr, SIZE_T GpuHandlePtr);
	~TD3DDescriptor();

	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;

	TD3DDescriptorHeap* Heap = nullptr;
};

class TD3DDescriptorHeap
{
public:
	TD3DDescriptorHeap();
	TD3DDescriptorHeap(TD3DDevice* InDevice, D3D12_DESCRIPTOR_HEAP_DESC Desc);
	~TD3DDescriptorHeap();

	TD3DDescriptor Alloc();

public:
	uint32_t DescriptorSize = 0;
	uint32_t DescriptorOffset = 0;
	uint32_t MaxDescriptorCount = 0;

	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Heap = nullptr;

	TD3DDevice* Device = nullptr;
};