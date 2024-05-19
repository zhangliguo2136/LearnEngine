#pragma once
#include <d3d12.h>
#include <stdint.h>
#include <list>
#include <vector>

#include "D3DDescriptor.h"

class TD3DDevice;

class TD3DDescriptorAllocator
{
public:
	struct HeapDescriptor
	{
		uint32_t HeapIndex;
		TD3DDescriptor Handle;
	};

private:
	struct FreeRange
	{
		SIZE_T CpuStart;
		SIZE_T CpuEnd;

		SIZE_T GpuStart;
		SIZE_T GpuEnd;
	};

	struct HeapEntry
	{
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeap = nullptr;
		std::list<TD3DDescriptorAllocator::FreeRange> FreeList;

		HeapEntry() {}
	};

public:
	TD3DDescriptorAllocator(TD3DDevice* InDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumDescriptorsPerHeap);
	~TD3DDescriptorAllocator();

	HeapDescriptor AllocateDescriptor();
	void FreeDescriptor(const HeapDescriptor& Descriptor);

private:
	D3D12_DESCRIPTOR_HEAP_DESC CreateHeapDesc(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumDescriptorsPerHeap);
	void AllocateHeap();

private:
	TD3DDevice* D3DDevice = nullptr;

	const D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	const uint32_t DescriptorSize;
	std::vector<HeapEntry> HeapMap;
};
