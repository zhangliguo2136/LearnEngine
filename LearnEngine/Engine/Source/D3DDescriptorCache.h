#pragma once
#include <map>
#include <vector>

#include "D3DDescriptor.h"

class TD3DDevice;

class TD3DDescriptorCache
{
public:
	TD3DDescriptorCache(TD3DDevice* InDevice);
	~TD3DDescriptorCache();

	void AppendDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, const std::vector<TD3DDescriptor>& Descriptors, TD3DDescriptor& OutDescriptor);

	TD3DDescriptorHeap GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type) { return HeapMap[Type]; }

private:
	TD3DDevice* Device = nullptr;
	std::map<D3D12_DESCRIPTOR_HEAP_TYPE, TD3DDescriptorHeap> HeapMap;

	static const int MaxCbvSrvUavDescripotrCount = 2048;
	static const int MaxRtvDescriptorCount = 1024;
};