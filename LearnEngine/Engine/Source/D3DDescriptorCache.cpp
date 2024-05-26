#include "D3DDescriptorCache.h"
#include "Utils.h"
#include "D3DDevice.h"

TD3DDescriptorCache::TD3DDescriptorCache(TD3DDevice* InDevice)
	:Device(InDevice)
{
	D3D12_DESCRIPTOR_HEAP_DESC SrvHeapDesc = {};
	SrvHeapDesc.NumDescriptors = 2048;
	SrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	TD3DDescriptorHeap SrvHeap(Device, SrvHeapDesc);
	HeapMap.insert({ SrvHeapDesc.Type, SrvHeap });

	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc = {};
	RtvHeapDesc.NumDescriptors = 1024;
	RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	TD3DDescriptorHeap RtvHeap(Device, RtvHeapDesc);
	HeapMap.insert({ RtvHeapDesc.Type, RtvHeap });
}

TD3DDescriptorCache::~TD3DDescriptorCache()
{
}

void TD3DDescriptorCache::AppendDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, const std::vector<TD3DDescriptor>& SrcDescriptors, TD3DDescriptor& OutDescriptor)
{
	auto& Heap = HeapMap[Type];

	uint32_t SlotsNeeded = (uint32_t)SrcDescriptors.size();

	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = { Heap.Heap->GetCPUDescriptorHandleForHeapStart().ptr + INT64(Heap.DescriptorOffset) * INT64(Heap.DescriptorSize) };
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> SrcCpuDescriptors;
	for (auto& Descriptor : SrcDescriptors)
	{
		SrcCpuDescriptors.push_back(Descriptor.CpuHandle);
	}
	Device->GetD3DDevice()->CopyDescriptors(1, &CpuHandle, &SlotsNeeded, SlotsNeeded, SrcCpuDescriptors.data(), nullptr, Type);

	OutDescriptor.CpuHandle = { Heap.Heap->GetCPUDescriptorHandleForHeapStart().ptr + INT64(Heap.DescriptorOffset) * INT64(Heap.DescriptorSize) };
	OutDescriptor.GpuHandle = { Heap.Heap->GetGPUDescriptorHandleForHeapStart().ptr + INT64(Heap.DescriptorOffset) * INT64(Heap.DescriptorSize) };

	Heap.DescriptorOffset += SlotsNeeded;
}

void TD3DDescriptorCache::Clear()
{
	for (auto& Pair : HeapMap)
	{
		auto& Heap = Pair.second;
		Heap.DescriptorOffset = 0;
	}
}
