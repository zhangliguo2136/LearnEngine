#pragma once
#include <d3d12.h>

#include "D3DDescriptorAllocator.h"

class TD3DDevice;
class TD3DResource;
class TD3DView
{
public:
	TD3DView(TD3DDevice* InDevice, TD3DResource* InResource, D3D12_DESCRIPTOR_HEAP_TYPE InHeapType);
	virtual ~TD3DView();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle() const { return { Descriptor.Handle.CpuHandle }; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle() const { return { Descriptor.Handle.GpuHandle }; }
	
	TD3DDescriptor GetDescriptor() { return Descriptor.Handle; }
private:
	TD3DDescriptorAllocator::HeapDescriptor Descriptor;

protected:
	TD3DDevice* Device = nullptr;
	TD3DResource* Resource = nullptr;

	TD3DDescriptorAllocator* DescriptorAllocator = nullptr;
	D3D12_DESCRIPTOR_HEAP_TYPE HeapType;
};

class TD3DShaderResourceView : public TD3DView
{
public:
	TD3DShaderResourceView(TD3DDevice* InDevice, TD3DResource* InResource);

	virtual ~TD3DShaderResourceView();
};

class TD3DUnorderedAccessView : public TD3DView
{
public:
	TD3DUnorderedAccessView(TD3DDevice* InDevice, TD3DResource* InResource);

	virtual ~TD3DUnorderedAccessView();
};

class TD3DRenderTargetView : public TD3DView
{
public:
	TD3DRenderTargetView(TD3DDevice* InDevice, TD3DResource* InResource);

	virtual ~TD3DRenderTargetView();
};

class TD3DDepthStencilView : public TD3DView
{
public:
	TD3DDepthStencilView(TD3DDevice* InDevice, TD3DResource* InResource);

	virtual ~TD3DDepthStencilView();
};