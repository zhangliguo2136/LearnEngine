#include "D3DDescriptorAllocator.h"

TD3DDescriptorAllocator::TD3DDescriptorAllocator(TD3DDevice* InDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumDescriptorsPerHeap)
	:D3DDevice(InDevice),
	HeapDesc(CreateHeapDesc(Type, NumDescriptorsPerHeap)),
	DescriptorSize(D3DDevice->GetD3DDevice()->GetDescriptorHandleIncrementSize(HeapDesc.Type))
{

}

TD3DDescriptorAllocator::~TD3DDescriptorAllocator()
{

}


TD3DDescriptorAllocator::HeapDescriptor TD3DDescriptorAllocator::AllocateDescriptor()
{
	int EntryIndex = -1;
	for (int i = 0; i < HeapMap.size(); i++)
	{
		if (HeapMap[i].FreeList.size() > 0)
		{
			EntryIndex = i;
			break;
		}
	}

	if (EntryIndex == -1)
	{
		AllocateHeap();
		EntryIndex = int(HeapMap.size() - 1);
	}

	HeapEntry& Entry = HeapMap[EntryIndex];

	FreeRange& Range = Entry.FreeList.front();

	HeapDescriptor Descriptor = { (uint32_t)EntryIndex, {Range.CpuStart ,Range.GpuStart} };

	Range.CpuStart += DescriptorSize;
	Range.GpuStart += DescriptorSize;
	if (Range.CpuStart == Range.CpuEnd)
	{
		Entry.FreeList.pop_front();
	}

	return Descriptor;
}

void TD3DDescriptorAllocator::FreeDescriptor(const HeapDescriptor& Descriptor)
{
	HeapEntry& Entry = HeapMap[Descriptor.HeapIndex];
	FreeRange NewRange =
	{
		Descriptor.Handle.CpuHandle.ptr,
		Descriptor.Handle.CpuHandle.ptr + DescriptorSize,

		Descriptor.Handle.GpuHandle.ptr,
		Descriptor.Handle.GpuHandle.ptr + DescriptorSize
	};

	bool bFound = false;
	for (auto Node = Entry.FreeList.begin(); Node != Entry.FreeList.end() && !bFound; Node++)
	{
		FreeRange& Range = *Node;

		if (Range.CpuStart < Range.CpuEnd)
		{
			Range.CpuStart = NewRange.CpuStart;
			Range.GpuStart = NewRange.GpuStart;
			bFound = true;
		}
		else if (Range.CpuEnd == NewRange.CpuStart)
		{
			Range.CpuEnd = NewRange.CpuEnd;
			Range.GpuEnd= NewRange.GpuEnd;
			bFound = true;
		}
		else
		{
			if (Range.CpuStart > NewRange.CpuStart)
			{
				Entry.FreeList.insert(Node, NewRange);
				bFound = true;
			}
		}
	}

	if (!bFound)
	{
		Entry.FreeList.push_back(NewRange);
	}
}

D3D12_DESCRIPTOR_HEAP_DESC TD3DDescriptorAllocator::CreateHeapDesc(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumDescriptorsPerHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
	Desc.Type = Type;
	Desc.NumDescriptors = NumDescriptorsPerHeap;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // This heap will not be bound to the shader
	Desc.NodeMask = 0;

	return Desc;
}

void TD3DDescriptorAllocator::AllocateHeap()
{
	// Create a new DescriptorHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Heap;
	ThrowIfFailed(D3DDevice->GetD3DDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Heap)));

	//Add an Entry covering the entire heap
	D3D12_CPU_DESCRIPTOR_HANDLE HeapCpuBase = Heap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE HeapGpuBase = Heap->GetGPUDescriptorHandleForHeapStart();

	HeapEntry Entry;
	Entry.DescriptorHeap = Heap;
	Entry.FreeList.push_back(
		{ 
			HeapCpuBase.ptr, 
			HeapCpuBase.ptr + (SIZE_T)HeapDesc.NumDescriptors * DescriptorSize,

			HeapGpuBase.ptr,
			HeapGpuBase.ptr + (SIZE_T)HeapDesc.NumDescriptors * DescriptorSize,
		}
	);

	HeapMap.push_back(Entry);
}
