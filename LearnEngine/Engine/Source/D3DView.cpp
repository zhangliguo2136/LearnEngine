#include "D3DView.h"

TD3DView::TD3DView(TD3DDevice* InDevice, TD3DResource* InResource, D3D12_DESCRIPTOR_HEAP_TYPE InHeapType)
	:Device(InDevice), Resource(InResource), HeapType(InHeapType)
{
	DescriptorAllocator = Device->GetDescriptorAllocator(InHeapType);
	
	if (DescriptorAllocator)
	{
		Descriptor = DescriptorAllocator->AllocateDescriptor();
	}
}

TD3DView::~TD3DView()
{
	if (DescriptorAllocator)
	{
		DescriptorAllocator->FreeDescriptor(Descriptor);
	}
}

TD3DShaderResourceView::TD3DShaderResourceView(TD3DDevice* InDevice, TD3DResource* InResource)
	:TD3DView(InDevice, InResource, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
{
}

TD3DShaderResourceView::~TD3DShaderResourceView()
{
}

TD3DUnorderedAccessView::TD3DUnorderedAccessView(TD3DDevice* InDevice, TD3DResource* InResource)
	:TD3DView(InDevice, InResource, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
{
}

TD3DUnorderedAccessView::~TD3DUnorderedAccessView()
{
}

TD3DRenderTargetView::TD3DRenderTargetView(TD3DDevice* InDevice, TD3DResource* InResource)
	:TD3DView(InDevice, InResource, D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
{
}

TD3DRenderTargetView::~TD3DRenderTargetView()
{
}

TD3DDepthStencilView::TD3DDepthStencilView(TD3DDevice* InDevice, TD3DResource* InResource)
	:TD3DView(InDevice, InResource, D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
{
}

TD3DDepthStencilView::~TD3DDepthStencilView()
{
}
