#include "D3DDescriptor.h"
#include "Utils.h"

TD3DDescriptor::TD3DDescriptor()
{

}

TD3DDescriptor::TD3DDescriptor(SIZE_T CpuHandlePtr, SIZE_T GpuHandlePtr)
	:CpuHandle({ CpuHandlePtr }), GpuHandle({GpuHandlePtr})
{
}

TD3DDescriptor::~TD3DDescriptor()
{
}

TD3DDescriptorHeap::TD3DDescriptorHeap()
{
}

TD3DDescriptorHeap::TD3DDescriptorHeap(TD3DDevice* InDevice, D3D12_DESCRIPTOR_HEAP_DESC Desc)
{
	Device = InDevice;
	HeapDesc = Desc;
	MaxDescriptorCount = Desc.NumDescriptors;

	DescriptorSize = Device->GetD3DDevice()->GetDescriptorHandleIncrementSize(Desc.Type);

	ThrowIfFailed(Device->GetD3DDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Heap)));
}

TD3DDescriptorHeap::~TD3DDescriptorHeap()
{
	Device = nullptr;
	Heap = nullptr;
}

TD3DDescriptor TD3DDescriptorHeap::Alloc()
{
	auto CpuHandle = Heap->GetCPUDescriptorHandleForHeapStart();
	auto GpuHandle = Heap->GetGPUDescriptorHandleForHeapStart();

	TD3DDescriptor Descritptor;
	Descritptor.Heap = this;
	Descritptor.CpuHandle.ptr = CpuHandle.ptr + DescriptorOffset * DescriptorSize;
	Descritptor.GpuHandle.ptr = GpuHandle.ptr + DescriptorOffset * DescriptorSize;

	DescriptorOffset++;

	return Descritptor;
}
