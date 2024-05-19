#pragma once
#include <d3d12.h>

#include "D3DDescriptorAllocator.h"

class TD3DView
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle() const { return { Descriptor.Handle.CpuHandle }; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle() const { return { Descriptor.Handle.GpuHandle }; }
	
private:
	TD3DDescriptorAllocator::HeapDescriptor Descriptor;
};

class TD3DShaderResourceView : public TD3DView
{

};

class TD3DUnorderedAccessView : public TD3DView
{

};

class TD3DRenderTargetView : public TD3DView
{

};

class TD3DDepthStencilView : public TD3DView
{

};